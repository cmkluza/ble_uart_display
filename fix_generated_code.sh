#!/bin/bash

# Constants
EDIT_START="// CMK: Start edit"
EDIT_END="// CMK: End edit"

# Main variable pairs
MAIN="Core/Src/main.c"
MAIN_TARGET_1="hsai_BlockB1.Instance = SAI1_Block_B;"
MAIN_APPEND_1="  // CMK: modified so asserts don't trigger - these are ignored in SPDIF mode anyways\n  hsai_BlockB1.Init.DataSize = SAI_DATASIZE_8;\n  hsai_BlockB1.FrameInit.FrameLength = 8;\n  hsai_BlockB1.FrameInit.ActiveFrameLength = 1;\n  hsai_BlockB1.SlotInit.SlotNumber = 1;"

# HAL time base variable pairs
HAL_TIMEBASE="Core/Src/stm32l5xx_hal_timebase_tim.c"
HAL_TARGET_1="HAL_NVIC_SetPriority(TIM1_UP_IRQn, TickPriority ,0);"
HAL_APPEND_1="  // CMK: fix error where code re-inits tick with invalid tick priority\n  uwTickPrio = TickPriority;"

# Append function - ${1} = target, ${2} = append, ${3} = file
function append {
    if ! grep -q "${2}" ${3}; then
        if ! grep -q "${1}" ${3}; then
            echo "Cannot find target in ${1}"
        else
            echo "Appending in ${3}"
            sed -i "/${1}/a${EDIT_START}\n${2}\n${EDIT_END}\n" ${3}
        fi
    else
        echo "Append already present in ${3}"
    fi
}

# Fix stuff in main.c
append "${MAIN_TARGET_1}" "${MAIN_APPEND_1}" "${MAIN}"

# Fix stuff in time base HAL file
append "${HAL_TARGET_1}" "${HAL_APPEND_1}" "${HAL_TIMEBASE}"

