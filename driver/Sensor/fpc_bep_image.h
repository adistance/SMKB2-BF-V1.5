/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_BEP_IMAGE_H
#define FPC_BEP_IMAGE_H

/**
 * @file    fpc_bep_image.h
 * @brief   Fingerprint image API
 *
 * This is the fingerprint image API of the Biometric Embedded Platform
 * (BEP) library. The library contains functionality for use with
 * biometric hardware from Fingerprint Cards. It is targeting embedded systems
 * with tight restrictions on available CPU, memory and storage resources.
 *
 * The library is by definition executing in the same security domain as the
 * caller, therefore the API does not define any security mechanisms and it is
 * the responsibility of the caller to securely deliver and protect any
 * sensitive data being delivered to other parts of the system.
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
 * @brief Allocates resources for a new image.
 *
 * @return The allocated image, or NULL if no resources are available.
 */
fpc_bep_image_t *fpc_bep_image_new(void);

/**
 * @brief Releases the provided image and the resources associated with it.
 *
 * @param[in] image The image to release. If NULL, no operation is performed.
 * Image pointer set to NULL when released.
 */
void fpc_bep_image_delete(fpc_bep_image_t **image);

/**
 * @brief Gets the size of the image pixels in number of bytes.
 *
 * @param[in] image The image.
 * @return Size of image pixels in number of bytes.
 */
size_t fpc_bep_image_get_size(const fpc_bep_image_t *image);

/**
 * @brief Gets the image pixels.
 *
 * Returns a reference to the image pixels. When the image has been
 * deleted using fpc_bep_image_delete() the pixels are no longer valid.
 *
 * The size of the returned pixels can be retrieved using
 * fpc_bep_get_image_size().
 *
 * @param[in] image The image.
 * @return The image pixels.
 */
uint8_t *fpc_bep_image_get_pixels(const fpc_bep_image_t *image);

#endif /* FPC_BEP_IMAGE_H */
