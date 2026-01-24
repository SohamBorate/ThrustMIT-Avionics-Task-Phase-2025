import numpy as np
from PIL import Image

image = np.array(Image.open("image.jpg"))
shape_y, shape_x, channels = image.shape

##########
# kernel weights
##########

size = 3 # should be odd
if size % 2 == 0:
    print("kernerl size invalid")
else:
    print("kernel size valid")

sigma = 1 # controls blur strength
kernel_weight = []
k = size // 2
e = 2.71828
pi = 3.14

for y in range(-k, k + 1):
    arr = []
    for x in range(-k, k + 1):
        # arr.append( (e ** ( -(x**2 + y**2) / (2 * sigma**2) )) )
        arr.append( (e ** ( -(x**2 + y**2) / (2 * sigma**2) )) / (2 * pi * sigma**2))
    print(arr)
    kernel_weight.append(arr)

kernel_weight_sum = np.array(kernel_weight).sum()

##########
# driver kernel thru image
##########

final_shape_x = shape_x - 2 * k
final_shape_y = shape_y - 2 * k
filtered_image = np.zeros((final_shape_y, final_shape_x, channels), dtype=np.uint8)

# set fitered image pixel values
for y in range(0, final_shape_y):
    for x in range(0, final_shape_x):
        for c in range(0, channels):
            final = 0
            for ky in range(-k, k + 1):
                for kx in range(-k, k + 1):
                    pixel = image[y + ky + k, x + kx + k, c]
                    weight = kernel_weight[ky + k][kx + k]
                    final += pixel * weight
            final /= kernel_weight_sum
            final = int(round(final))
            if (final < 0):
                final = 0
            elif final > 255:
                final = 255
            filtered_image[y, x, c] = final

out_img = Image.fromarray(filtered_image, mode="RGB")
out_img.save("gaussian.jpg", format="JPEG")
