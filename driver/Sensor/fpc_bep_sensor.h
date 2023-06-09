/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_BEP_SENSOR_H
#define FPC_BEP_SENSOR_H

/**
 * @file    fpc_bep_sensor.h
 * @brief   Fingerprint sensor API
 *
 * This is the fingerprint sensor API of the Biometric Embedded Platform
 * (BEP) library. The library contains functionality for use with
 * biometric hardware from Fingerprint Cards. It is targeting embedded systems
 * with tight restrictions on available CPU, memory and storage resources.
 *
 * The library is by definition executing in the same security domain as the
 * caller, therefore the API does not define any security mechanisms and it is
 * the responsibility of the caller to securely deliver and protect any
 * sensitive data being delivered to other parts of the system.
 *
 * @note This is a work-in-progress specification. Implementers are informed
 * that this API may change without providing any backward compatibility.
 * However it is FPC's ambition that the API shall remain compatible between
 * releases.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "fpc_bep_types.h"
#include "fpc_bep_sensors.h"

/**
 * @brief Properties for fingerprint sensor.
 */
typedef struct {
    fpc_bep_sensor_type_t sensor_type; ///< Configured sensor type.
    uint16_t width;                    ///< Horizontal number of pixels of sensor HW.
    uint16_t height;                   ///< Vertical number of pixels of sensor HW.
    uint16_t dpi;                      ///< Resolution in Dots Per Inch of sensor HW.
    uint32_t max_spi_clock;            ///< Max SPI speed.
    uint8_t  num_sub_areas_width;      ///< Number of sub-areas for finger detection horizontally.
    uint8_t  num_sub_areas_height;     ///< Number of sub-areas for finger detection vertically.
} fpc_bep_sensor_prop_t;

/**
 * @brief Configurable parameters for FPC BEP sensor functions
 */
typedef struct {
    uint8_t nbr_of_finger_present_zones;                ///< Number of detect zones for finger
                                                        ///< present status.
    fpc_bep_sensor_driver_mechanism_t driver_mechanism; ///< Interrupt is default.
    fpc_bep_sensor_reset_t reset;                       ///< Hard reset is default.
} fpc_bep_sensor_param_t;

/**
 * @brief Gets the sensor type for a specific sensor.
 *
 * This function retrieves the sensor type based on a sensor configuration.
 * It could be used before the sensor is initialized.
 *
 * @param[in] sensor The fingerprint sensor configuration.
 * @return ::fpc_bep_sensor_type_t
 */
fpc_bep_sensor_type_t fpc_bep_sensor_get_type(const fpc_bep_sensor_t *sensor);

/**
 * @brief Gets recommended sensor function configuration parameters for the FPC BEP library.
 *
 * This function could be called before the sensor is initialized.
 *
 * @param[in] sensor Sensor configuration.
 * @param[out] param The recommended BEP library sensor function configuration parameters.
 *
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t fpc_bep_sensor_get_recommended_param(const fpc_bep_sensor_t *sensor,
                                                      fpc_bep_sensor_param_t *param);


/**
 * @brief Initializes the fingerprint sensor part of FPC BEP library.
 *
 * When returning from this function the sensor is in deep sleep mode.
 *
 * @param[in] sensor The fingerprint sensor configuration to be used with the BEP library. It
 * can be any of the declared fpc_bep_sensor_x types in file fpc_bep_sensors.h
 * (eg fpc_bep_sensor_1020...).
 * @param[in] calibration Sensor calibration data. If not NULL, a reference to
 * the supplied buffer is stored. Caller must not delete buffer. If NULL,
 * sensor calibration is run as part of fpc_bep_sensor_init() and calibration
 * data is stored internally.
 * @param[in] params Optional sensor function configuration parameters. NULL for default.
 *
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t fpc_bep_sensor_init(const fpc_bep_sensor_t *sensor, const uint8_t *calibration,
        const fpc_bep_sensor_param_t *param);

/**
 * @brief Releases the fingerprint sensor part of FPC BEP library.
 *
 * After sensor resources successfully has been released a new
 * initialization must be performed by calling fpc_bep_sensor_init().
 *
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t fpc_bep_sensor_release(void);

/**
 * @brief Activates the sensor sleep mode.
 *
 * This function will order the sensor to go to sleep mode. In sleep mode the
 * sensor will wake up periodically to perform reduced finger queries and then
 * go back to sleep in order to save energy. Once a finger is detected, an
 * interrupt is generated by the sensor and a status flag is set. The finger
 * present status can be polled by calling function fpc_bep_finger_detect.
 *
 * @param[in] sleep_time Nominal sleep time between finger present queries [ms].
 * Range [0, 1020]. Resolution 4ms.
 * The clock frequency is guaranteed to be in [-50%, +19%] of nominal.
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t fpc_bep_sensor_sleep(uint16_t sleep_time);

/**
 * @brief Gets the result of the latest reduced finger present query.
 *
 * This function will check the interrupt status flag. If the flag is
 * set, it will read and clear the sensor interrupt status register.
 *
 * @return ::true if finger is detected, false otherwise.
 */
bool fpc_bep_finger_detect(void);

/**
 * @brief Activates the sensor deep sleep mode.
 *
 * This function will order the sensor to go to deep sleep mode.
 *
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t fpc_bep_sensor_deep_sleep(void);

/**
 * @brief Checks if there is a finger present on the sensor.
 *
 * This function will put the sensor in idle mode, query the sensor for finger
 * present status and then finally put the sensor in deep sleep mode.
 * The number of finger detect zones can be configured with the fpc_bep_sensor_param_t
 * in fpc_bep_sensor_init.
 *
 * @param[out] finger_status Finger present status.
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t fpc_bep_check_finger_present(fpc_bep_finger_status_t *finger_status);

/**
 * @brief Captures an image from the sensor.
 *
 * This function will put the sensor in idle mode and then capture an image.
 * If the image object references previously captured data it will be overwritten with the newly
 * captured data. On successful return the image can be used as input for enrollment or
 * identification. Once the image has been captured the sensor is put in deep sleep mode.
 *
 * Note that the sensor should be covered sufficiently by the finger prior to calling this
 * function. Use function ::fpc_bep_check_finger_present to query the sensor coverage.
 *
 * @param[in,out] image The captured image.
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t fpc_bep_capture(fpc_bep_image_t *image);

/**
 * @brief Resets the sensor.
 *
 * This function performs a hard reset of the sensor, configures the sensor
 * registers with recommended settings and finally puts the sensor in
 * deep sleep mode.
 *
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t fpc_bep_sensor_reset(void);

/**
 * @brief Gets the sensor properties.
 *
 * This function retrieves the sensor properties.
 *
 * @param[out] properties The retrieved sensor properties.
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t fpc_bep_sensor_properties(fpc_bep_sensor_prop_t *properties);

/**
 * @brief Signals that actual image capture is started, after finger detect.
 *
 * The purpose of this function is to support time measurement of image capture
 * without including time waiting for finger. The definition of this function is
 * weak in the BEP library. Redefine function outside of the library where
 * needed.
 */
void fpc_bep_signal_image_capture_start(void);

#endif /* FPC_BEP_SENSOR_H */
