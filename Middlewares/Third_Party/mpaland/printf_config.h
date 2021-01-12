/*
 * printf_config.h
 *
 *  Created on: Jan 12, 2021
 *      Author: Cameron Kluza
 */

#ifndef THIRD_PARTY_MPALAND_PRINTF_CONFIG_H_
#define THIRD_PARTY_MPALAND_PRINTF_CONFIG_H_

/* I only need a little bit of the printf support, disable most of it */
#define PRINTF_DISABLE_SUPPORT_FLOAT
#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL
#define PRINTF_DISABLE_SUPPORT_LONG_LONG

#endif /* THIRD_PARTY_MPALAND_PRINTF_CONFIG_H_ */
