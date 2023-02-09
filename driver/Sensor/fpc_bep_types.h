/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_BEP_TYPES_H
#define FPC_BEP_TYPES_H

#include <stdbool.h>

/**
 * @file    fpc_bep_types.h
 * @brief   Biometric Embedded Platform types.
 *
 * This is the common types used by Biometric Embedded Platform (BEP) library.
 *
 * @note This is a work-in-progress specification. Implementers are informed
 * that this API may change without providing any backward compatibility.
 * However it is FPC's ambition that the API shall remain compatible between
 * releases.
 */

/** @brief Common results returned by BEP functions.
 *
 * \par BEP config/usage errors:
 * <b>Examples:</b> Incorrect arguments/parameters when calling BEP API
 * functions; functions called in incorrect order. <br>
 * <b>Action:</b> Fix SW bug. <br>
 * \par
 * FPC_BEP_RESULT_GENERAL_ERROR <br>
 * FPC_BEP_RESULT_NOT_IMPLEMENTED <br>
 * FPC_BEP_RESULT_NOT_SUPPORTED <br>
 * FPC_BEP_RESULT_NOT_INITIALIZED <br>
 * FPC_BEP_RESULT_CANCELLED <br>
 * FPC_BEP_RESULT_NO_RESOURCE <br>
 * FPC_BEP_RESULT_WRONG_STATE <br>
 * FPC_BEP_RESULT_ID_NOT_UNIQUE <br>
 * FPC_BEP_RESULT_ID_NOT_FOUND <br>
 * FPC_BEP_RESULT_INVALID_FORMAT <br>
 * FPC_BEP_RESULT_INVALID_ARGUMENT <br>
 * FPC_BEP_RESULT_INVALID_PARAMETER <br>
 * FPC_BEP_RESULT_INVALID_CALIBRATION <br>
 * FPC_BEP_RESULT_MISSING_TEMPLATE <br>
 * FPC_BEP_RESULT_STORAGE_NOT_FORMATTED <br>
 * FPC_BEP_RESULT_SENSOR_NOT_INITIALIZED <br>
 * FPC_BEP_RESULT_SENSOR_MISMATCH <br>
 * FPC_BEP_RESULT_CRYPTO_ERROR
 *
 * \par Dynamic memory/heap errors:
 * <b>Examples:</b> Memory leak; heap is too small. <br>
 * <b>Action:</b> Fix SW bug. <br>
 * \par
 * FPC_BEP_RESULT_NO_MEMORY
 *
 * \par Sensor and communication errors:
 * <b>Examples:</b> Broken sensor communication lines; unstable power supply. <br>
 * <b>Action:</b> Fix HW bug. <br>
 * \par
 * FPC_BEP_RESULT_BROKEN_SENSOR <br>
 * FPC_BEP_RESULT_INTERNAL_ERROR <br>
 * FPC_BEP_RESULT_TIMEOUT <br>
 * FPC_BEP_RESULT_IO_ERROR
 *
 * \par Image capture errors:
 * <b>Examples:</b> Finger is not stable; finger removed from sensor too quickly. <br>
 * <b>Action:</b> Call the function again. <br>
 * \par
 * FPC_BEP_FINGER_NOT_STABLE <br>
 * FPC_BEP_RESULT_IMAGE_CAPTURE_ERROR <br>
 * FPC_BEP_RESULT_TOO_MANY_BAD_IMAGES
 */

typedef enum {
    /** No errors occurred. */
    FPC_BEP_RESULT_OK = 0,
    /** General error. */
    FPC_BEP_RESULT_GENERAL_ERROR = -1,
    /** Internal error. */
    FPC_BEP_RESULT_INTERNAL_ERROR = -2,
    /** Invalid argument. */
    FPC_BEP_RESULT_INVALID_ARGUMENT = -3,
    /** The functionality is not implemented. */
    FPC_BEP_RESULT_NOT_IMPLEMENTED = -4,
    /** The operation was cancelled. */
    FPC_BEP_RESULT_CANCELLED = -5,
    /** Out of memory. */
    FPC_BEP_RESULT_NO_MEMORY = -6,
    /** Resources are not available. */
    FPC_BEP_RESULT_NO_RESOURCE = -7,
    /** An I/O error occurred. */
    FPC_BEP_RESULT_IO_ERROR = -8,
    /** Sensor is broken. */
    FPC_BEP_RESULT_BROKEN_SENSOR = -9,
    /** The operation cannot be performed in the current state. */
    FPC_BEP_RESULT_WRONG_STATE = -10,
    /** The operation timed out. */
    FPC_BEP_RESULT_TIMEOUT = -11,
    /** The ID is not unique. */
    FPC_BEP_RESULT_ID_NOT_UNIQUE = -12,
    /** The ID is not found. */
    FPC_BEP_RESULT_ID_NOT_FOUND = -13,
    /** The format is invalid. */
    FPC_BEP_RESULT_INVALID_FORMAT = -14,
    /** An image capture error occurred. */
    FPC_BEP_RESULT_IMAGE_CAPTURE_ERROR = -15,
    /** Sensor hardware id or sensor configuration mismatch. */
    FPC_BEP_RESULT_SENSOR_MISMATCH = -16,
    /** Invalid parameter. */
    FPC_BEP_RESULT_INVALID_PARAMETER = -17,
    /** Missing Template. */
    FPC_BEP_RESULT_MISSING_TEMPLATE = -18,
    /** Invalid Calibration.*/
    FPC_BEP_RESULT_INVALID_CALIBRATION = -19,
    /** Calibration/template storage not formatted.*/
    FPC_BEP_RESULT_STORAGE_NOT_FORMATTED = -20,
    /** Sensor hasn't been initialized. */
    FPC_BEP_RESULT_SENSOR_NOT_INITIALIZED = -21,
    /** Enroll fail after too many bad images. */
    FPC_BEP_RESULT_TOO_MANY_BAD_IMAGES = -22,
    /** Cryptographic operation failed. */
    FPC_BEP_RESULT_CRYPTO_ERROR = -23,
    /** The functionality is not supported. */
    FPC_BEP_RESULT_NOT_SUPPORTED = -24,
    /** Finger not stable during image capture. */
    FPC_BEP_FINGER_NOT_STABLE = -25,
    /** The functionality could not be used before it's initialized. */
    FPC_BEP_RESULT_NOT_INITIALIZED = -26,
} fpc_bep_result_t;

/**
 * @brief Fingerprint sensor type enumeration.
 */
typedef enum {
    FPC_BEP_SENSOR_TYPE_UNDEFINED = 0,
    FPC_BEP_SENSOR_TYPE_RESERVED1 = 2,
    FPC_BEP_SENSOR_TYPE_FPC1261   = 9,
} fpc_bep_sensor_type_t;

/**
 * @brief Finger present status.
 */
typedef enum {
    /** Undefined value */
    FPC_BEP_FINGER_STATUS_UNDEFINED   = 0,
    /** Sufficient number of sensor sub-areas are covered */
    FPC_BEP_FINGER_STATUS_PRESENT     = 1,
    /** No sensor sub-areas are covered */
    FPC_BEP_FINGER_STATUS_NOT_PRESENT = 2,
    /** Sufficient number of sensor sub-areas are not covered */
    FPC_BEP_FINGER_STATUS_PARTIAL     = 3,
} fpc_bep_finger_status_t;

/**
 * @brief FPC sensor driver mechanism.
 */
typedef enum {
    /** Sensor HW interrupt. */
    FPC_BEP_INTERRUPT_DRIVEN = 0,
    /** Sensor polling. */
    FPC_BEP_POLLING = 1
} __attribute__((packed)) fpc_bep_sensor_driver_mechanism_t;

/**
 * @brief FPC sensor reset type.
 */
typedef enum {
    /** Hard reset via sensor reset signal. */
    FPC_BEP_HARD_RESET = 0,
    /** Soft reset via sensor SPI command.  */
    FPC_BEP_SOFT_RESET = 1
} __attribute__((packed)) fpc_bep_sensor_reset_t;

/**
 * @brief Fingerprint common configuration type.
 */
typedef enum {
    FPC_BEP_CONFIG_DISABLE = 0,
    FPC_BEP_CONFIG_ENABLE  = 1
} __attribute__((packed)) fpc_bep_config_t;

/**
 * @brief Image used to store a captured image.
 */
typedef struct fpc_bep_image fpc_bep_image_t;

/**
 * @brief Sensor
 */
typedef struct fpc_bep_sensor fpc_bep_sensor_t;

/**
 * @brief Function that returns a bool deciding if WFI mode should be entered or not.
 * @return true enter WFI, false don't enter WFI
 */
typedef bool (*fpc_bep_wfi_check_t)(void);

#endif /* FPC_BEP_TYPES_H */
