R"(
	__kernel void jpngToPng(__read_only image2d opacityLayer, __read_only image2d colorLayer, __write_only resultImage)
	{
		int2 coord = (int2)(get_global_id(0), get_global_id(1));

		uint4 opacity = read_imageui(opacityLayer, coord);
		uint4 color = read_imageui(colorLayer, coord);

		uchar* bytesOpacity = (uchar*)&opacity;
		uchar* bytesColor = (uchar*)&color;

		uint4 mix = (uint4)(bytesOpacity[1], bytesColor[1], bytesColor[2], bytesColor[3]);

		write_imageui(resultImage, mix);
	}
)"