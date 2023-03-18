#ifndef INC_IMAGES_H_
#define INC_IMAGES_H_

#include <stdint.h>

typedef struct DotMatrixImage {
	const uint8_t width;
	const uint8_t height;
	const uint8_t *image_data;
} DotMatrixImage;

#include "images/imageexterns.h"

#endif /* INC_IMAGES_H_ */
