R"(
	__kernel void jpngToPng(__read_only image2d opacityLayer, __read_only image2d colorLayer, __write_only resultImage)
	{
		int2 coord = (int2)(get_global_id(0), get_global_id(1));

		uchar4 opacity = read_imageui(opacityLayer, coord);
		uchar4 color = read_imageui(colorLayer, coord);

		uchar4 mix = (uchar4)(opacity[1], color[1], color[2], color[3]);

		write_imageui(resultImage, mix);
	}
)"