/*
 * ble.cpp
 *
 * Module for interacting with BlueNRG module using the STM CubeMX interface.
 *
 * Copyright (c) 2020 Cameron Kluza
 * Distributed under the MIT license (see LICENSE or https://opensource.org/licenses/MIT)
 */

#include "ble.h"

extern "C" {
#   include <stm32l5xx_hal.h>

#   include <bluenrg_conf.h>
#   include <bluenrg_gap.h>
#   include <bluenrg_gap_aci.h>
#   include <bluenrg_gatt_aci.h>
#   include <bluenrg_hal_aci.h>
#   include <bluenrg_utils.h>
#   include <hci.h>
#   include <hci_le.h>
#   include <sm.h>
}

#include <etl/vector.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace ble {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Types
////////////////////////////////////////////////////////////////////////////////////////////////////

enum class State {
    IDLE,
    ADVERTISING,
    SCANNING,
    CONNECTED,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Data
////////////////////////////////////////////////////////////////////////////////////////////////////
/** Callback state. */
static event_callback g_callbacks[CALLBACK_COUNT] {};
static void *g_contexts[CALLBACK_COUNT] {};
static std::uint32_t g_callback_cnt {};

/** BLE state. */
static std::atomic<State> g_state {};

/** Configured BLE role. */
static Role g_ble_role {};

/** Expansion board version. */
static ExpansionBoard g_expansion_board { ExpansionBoard::UNKNOWN };

/** Advertising state. */
static etl::vector<char, 31> g_adv_name {};
static etl::vector<std::uint8_t, 31> g_adv_uuid16s {};
static etl::vector<std::uint8_t, 31> g_adv_uuid128s {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Prototypes
////////////////////////////////////////////////////////////////////////////////////////////////////

/** Passes callback info to registered event handlers. */
static void process_aci_packet(void *data);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public Implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

bool init(Role role) {
    g_ble_role = role;

	/** Initialize the host controller interface. */
	hci_init(process_aci_packet, NULL);

	/* Get the BlueNRG HW and FW versions. */
	uint8_t  hwVersion;
	uint16_t fwVersion;
	getBlueNRGVersion(&hwVersion, &fwVersion);
	printf("HWver %d, FWver %d\n", hwVersion, fwVersion);
    if (hwVersion > 0x30) { /* X-NUCLEO-IDB05A1 expansion board is used */
        g_expansion_board = ExpansionBoard::IDB05A1;
    } else {
        g_expansion_board = ExpansionBoard::IDB04A1;
    }

	/*
	 * Reset BlueNRG again otherwise we won't
	 * be able to change its MAC address.
	 * aci_hal_write_config_data() must be the first
     * command after reset otherwise it will fail.
	 */
	hci_reset();
	HAL_Delay(100);

	std::uint8_t bdaddr[BDADDR_SIZE] = { 0x00, 0x00, 0x00, 0xE1, 0x80, 0x02 };
	if (g_ble_role == Role::CLIENT) {
	    bdaddr[0] = 0xbb;
	} else { /* Server */
	    bdaddr[0] = 0xaa;
	}

	auto ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
	                                     CONFIG_DATA_PUBADDR_LEN,
                                         bdaddr);

	if (ret) {
	    printf("%s: Set public address failed: %02X\n", __func__, ret);
	    return false;
	}

	ret = aci_gatt_init();
	if (ret) {
	    printf("%s: GATT init failed: %02X\n", __func__, ret);
	    return false;
	}

	/* Determine the GAP role based on BLE role and expansion board. */
	std::uint8_t gap_role;
	if (g_ble_role == Role::SERVER) {
        if (g_expansion_board == ExpansionBoard::IDB04A1) {
            gap_role = GAP_PERIPHERAL_ROLE_IDB04A1;

        } else {
            gap_role = GAP_PERIPHERAL_ROLE_IDB05A1;
        }
	} else { /* Client */
        if (g_expansion_board == ExpansionBoard::IDB04A1) {
            gap_role = GAP_CENTRAL_ROLE_IDB04A1;
        } else {
            gap_role = GAP_CENTRAL_ROLE_IDB05A1;
        }
	}

	/* Perform GAP initialization for the proper expansion board. */
	std::uint16_t service_handle;
    std::uint16_t dev_name_char_handle;
    std::uint16_t appearance_char_handle;
    if (g_expansion_board == ExpansionBoard::IDB04A1) {
        ret = aci_gap_init_IDB04A1(gap_role, &service_handle,
                                   &dev_name_char_handle,
                                   &appearance_char_handle);
    } else {
        ret = aci_gap_init_IDB05A1(gap_role, 0, 0x07, &service_handle,
                                   &dev_name_char_handle,
                                   &appearance_char_handle);
    }

    if (ret != BLE_STATUS_SUCCESS) {
        printf("%s: GAP init failed: %02X\n", __func__, ret);
        return false;
    }

    ret = aci_gap_set_auth_requirement(
            MITM_PROTECTION_REQUIRED,  /* MITM mode: enabled */
            OOB_AUTH_DATA_ABSENT,      /* Out-of-band data: disabled */
            nullptr,                   /* OOB data buffer: unused */
            7,                         /* Minimum encryption key size */
            16,                        /* Maximum encryption key size */
            USE_FIXED_PIN_FOR_PAIRING, /* Use fixed pin for pairing: yes */
            123456,                    /* Fixed pairing pin: TODO change */
            BONDING);                  /* Bonding: enabled */

    if (ret != BLE_STATUS_SUCCESS) {
        printf("%s: GAP set auth failed: %02X\n", __func__, ret);
        return false;
    }

    /* Set output power level. */
    ret = aci_hal_set_tx_power_level(1, 4);
    if (ret != BLE_STATUS_SUCCESS) {
        printf("%s: Set TX power failed: %02X\n", __func__, ret);
        return false;
    }

    printf("%s: Successfully setup BLE\n", __func__);
    return true;
}

ExpansionBoard board() {
    return g_expansion_board;
}

namespace advertising {

    bool start() {
        if (g_state == State::ADVERTISING) {
            return true; // already advertising TODO: need actual error codes
        }

        hci_le_set_scan_resp_data(0, nullptr);

        const auto name_adv_len = g_adv_name.size();
        const auto uuid16_adv_len = g_adv_uuid16s.size();
        const auto uuid128_adv_len = g_adv_uuid128s.size();

        if (name_adv_len + uuid16_adv_len + uuid128_adv_len > 31) {
            printf("%s: too many bytes in advertising packet: %d",
                    __func__, name_adv_len + uuid16_adv_len + uuid128_adv_len);
            return false;
        }

        const char *name { nullptr };
        std::uint8_t name_len {};
        std::uint8_t uuids[31] {};
        std::uint8_t uuid_len {};

        if (name_adv_len != 0) {
            name = g_adv_name.data();
            name_len = static_cast<std::uint8_t>(name_adv_len);
        }

        if (uuid16_adv_len != 0) {
            std::memcpy(&uuids[uuid_len], g_adv_uuid16s.data(), uuid16_adv_len);
            uuid_len = static_cast<std::uint8_t>(uuid16_adv_len + uuid_len);
        }

        if (uuid128_adv_len != 0) {
            std::memcpy(&uuids[uuid_len], g_adv_uuid128s.data(), uuid128_adv_len);
            uuid_len = static_cast<std::uint8_t>(uuid128_adv_len + uuid_len);
        }

        auto ret = aci_gap_set_discoverable(
                 ADV_DATA_TYPE,     /* Advertisement type */
                 ADV_INTERV_MIN,    /* Min advertising interval */
                 ADV_INTERV_MAX,    /* Max advertising interval */
                 PUBLIC_ADDR,       /* Address type */
                 NO_WHITE_LIST_USE, /* Filter policy */
                 name_len,          /* Length of local name array */
                 name,              /* Local name array */
                 uuid_len,          /* Length of UUID array */
                 uuids,             /* UUID array */
                 0,                 /* Min slave connection interval */
                 0);                /* Max slave connection interval */

        if (ret != BLE_STATUS_SUCCESS) {
            printf("%s: Set discoverable failed: %02X\n", __func__, ret);
            return false;
        }

        g_state = State::ADVERTISING;

        return true;
    }

    void stop() {
        if (g_state != State::ADVERTISING) {
            return;
        }

        auto ret = aci_gap_set_non_discoverable();
        if (ret != BLE_STATUS_SUCCESS) {
            printf("%s: Set non-discoverable failed: %02X\n", __func__, ret);
        }

        g_state = State::IDLE;
    }

    void clear() {
        g_adv_name.clear();
        g_adv_uuid16s.clear();
        g_adv_uuid128s.clear();
    }

    bool add_name(const char *name) {
        auto len = std::strlen(name);
        if (len > 30) {
            printf("%s: Name too long (%d > 30)\n", __func__, len);
            return false;
        }

        g_adv_name.push_back(AD_TYPE_COMPLETE_LOCAL_NAME);
        g_adv_name.insert(g_adv_name.begin() + 1, name, name + len);

        return true;
    }

    bool add_uuid16(std::uint16_t uuid) {
        if (g_adv_uuid16s.size() + 2 > 31) {
            printf("%s: Too many UUIDs (%d bytes)\n",
                    __func__, g_adv_uuid16s.size() + 2);
            return false;
        }

        if (g_adv_uuid16s.empty()) {
            g_adv_uuid16s.push_back(AD_TYPE_16_BIT_SERV_UUID);
        }

        // TODO: swap?
        g_adv_uuid16s.push_back(static_cast<std::uint8_t>((uuid >> 0) & 0xFF));
        g_adv_uuid16s.push_back(static_cast<std::uint8_t>((uuid >> 8) & 0xFF));
        return true;
    }

    bool add_uuid128(const std::uint8_t *uuid) {
        if (g_adv_uuid128s.size() + 16 > 31) {
            printf("%s: Too many UUIDs (%d bytes)\n",
                    __func__, g_adv_uuid128s.size() + 16);
            return false;
        }

        if (g_adv_uuid128s.empty()) {
            g_adv_uuid128s[0] = AD_TYPE_128_BIT_SERV_UUID;
        }

        // TODO: swap?
        for (int i = 0; i < 8; ++i) {
            g_adv_uuid16s.push_back(uuid[i]);
        }
        return true;
    }

} /* namespace advertising */

void register_callback(event_callback callback, void *context) {
	if (g_callback_cnt < CALLBACK_COUNT) {
		g_callbacks[g_callback_cnt] = callback;
		g_contexts[g_callback_cnt] = context;
		++g_callback_cnt;
	}
}

void process_events() {
    hci_user_evt_proc();
}

void thread(void *arg) {
    (void) arg; // TODO: is there an unused macro around?

    while (true) {
        process_events();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: update state
static void process_aci_packet(void *data) {
    auto *event = reinterpret_cast<hci_uart_pckt *>(data);

	for (std::uint32_t i = 0; i < g_callback_cnt; ++i) {
		g_callbacks[i](g_contexts[i], event);
	}

	if (event->type != HCI_EVENT_PKT) {
	    return;
	}

	auto *hci_packet = reinterpret_cast<hci_event_pckt *>(event->data);

	switch (hci_packet->evt) {

	    case EVT_DISCONN_COMPLETE: {
	        printf("Disconnected\n");
	    } break;

	    case EVT_LE_META_EVENT: {
            auto *le_event = reinterpret_cast<evt_le_meta_event *>(hci_packet->data);

	        switch (le_event->subevent) {

	            case EVT_LE_CONN_COMPLETE: {
	                auto *conn_event =
	                        reinterpret_cast<evt_le_connection_complete *>(le_event->data);
	                auto *addr = conn_event->peer_bdaddr;
	                printf("Connected to: %02X:%02X:%02X:%02X:%02X:%02X (%d)\n",
	                        addr[5], addr[4], addr[3], addr[2], addr[1], addr[0],
	                        conn_event->handle);
	            } break;

	        }

	    } break;

	}
}

} /* namespace ble */
