#include "rtweekend.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

#include<iostream>
#include<SDL.h>

color ray_color(const ray& r, const hittable& world, int depth) {
	hit_record rec;

	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return color(0, 0, 0);

	if (world.hit(r, 0.001, infinity, rec)) {
		ray scattered;
		color attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, depth - 1);
		return color(0, 0, 0);
	}
	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

void SaveScreenshot(int image_width, int image_height, SDL_Renderer* renderer)
{
	const Uint32 format = SDL_PIXELFORMAT_ARGB8888;
	const int width = image_width;
	const int height = image_height;
	//auto renderer = sdl2Core->GetRenderer();

	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, format);
	SDL_RenderReadPixels(renderer, NULL, format, surface->pixels, surface->pitch);
	SDL_SaveBMP(surface, "screenshot.bmp");
	SDL_FreeSurface(surface);
}

int main(int argc, char* args[]) {
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;

	//Image
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 400;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 100;
	const int max_depth = 50;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(image_width, image_height, 0, &window, &renderer);

	// World
	hittable_list world;

	auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
	auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
	auto material_left = make_shared<dielectric>(1.5);
	auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

	world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
	world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), -0.45, material_left));
	world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));
	
	// Camera
	point3 lookfrom(3, 3, 2);
	point3 lookat(0, 0, -1);
	vec3 vup(0, 1, 0);
	auto dist_to_focus = (lookfrom - lookat).length();
	auto aperture = 2.0;

	camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

	// Render
	
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	for (int j = image_height - 1; j >= 0; --j) {
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i) {
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s) {
				auto u = (i + random_double()) / (image_width - 1);
				auto v = (j + random_double()) / (image_height - 1);
				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, world, max_depth);
			}
			color converted_color = write_color(pixel_color, samples_per_pixel);
			SDL_SetRenderDrawColor(renderer, converted_color.x(), converted_color.y(), converted_color.z(), 255);
			SDL_RenderDrawPoint(renderer, i, image_height-1-j);

			//std::cout << ir << ' ' << ig << ' ' << ib << '\n';
		}
	}

	//SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	//SDL_RenderDrawPoint(renderer, image_width / 2, image_height / 2);

	SDL_RenderPresent(renderer);
	SaveScreenshot(image_width, image_height, renderer);
	SDL_Delay(100000);
	
	return 0;
}