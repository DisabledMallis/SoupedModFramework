R"(
	__kernel void jpngToPng(__read_only image2d opacityLayer, __read_only image2d colorLayer, __write_only resultImage)
	{
		int x = get_global_id(0);
		int y = get_global_id(1);


	}
)"