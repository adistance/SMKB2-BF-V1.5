/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_BEP_CALIBRATION_H
#define FPC_BEP_CALIBRATION_H

/**
 * @file    fpc_bep_calibration.h
 * @brief   Biometric Embedded Platform sensor calibration.
 *
 * This is the sensor calibration API of the Biometric Embedded Platform (BEP)
 * library.
 *
 * @note Before using this API the sensor must be initialized, see
 * fpc_bep_sensor_init().
 *
 * @note This is a work-in-progress specification. Implementers are informed
 * that this API may change without providing any backward compatibility.
 * However it is FPC's ambition that the API shall remain compatible between
 * releases.
 */

#include <stdint.h>

#include "fpc_bep_types.h"

/**
 * @brief Gets a reference to the serialized sensor calibration data produced during previous call
 * of fpc_bep_sensor_init().
 *
 * @param[out] calibration Serialized calibration data.
 * @param[out] size Size of serialized data [byte].
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t fpc_bep_cal_get(uint8_t **calibration, size_t *size);

#endif /* FPC_BEP_CALIBRATION_H */
