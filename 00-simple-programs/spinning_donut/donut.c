#include<stdio.h>
#include<string.h>
#include<math.h>
#include<threads.h>

#define WIDTH 60
#define HEIGHT 20
#define PI 3.14159265f
#define RADIUS_1 1.0f      // MINOR RADIUS (R1)
#define RADIUS_2 2.0f     // MAJOR RADIUS (R2)
#define Z 10.0f           // Z_offset
#define FOV 25.0f       // Scale Factor (K1)
#define STEP_1 0.07f   // step for angle theta
#define STEP_2 0.02f  // step for angle phi
#define L_x 0.0f // CHOOSING A LIGHT SOURCE VECTOR
#define L_y 1.0f
#define L_z -1.0f
				

void rotateX(float *x, float *y, float *z, float cos_alpha, float sin_alpha) {
    // float x_ = *x;
    float y_ = *y;
    float z_ = *z;

    // *x = x_;
    *y = y_ * cos_alpha - z_ * sin_alpha;
    *z = z_ * cos_alpha + y_ * sin_alpha;
}

/*
void rotateY(float *x, float *y, float *z, float cos_alpha, float sin_alpha) {
    float x_ = *x;
    // float y_ = *y;
    float z_ = *z;

    *x = x_ * cos_alpha - z_ * sin_alpha;
    // *y = y_;
    *z = -(x_) * cos_alpha + z_ * sin_alpha;
}
*/

void rotateZ(float *x, float *y, float *z, float cos_alpha, float sin_alpha) {
    float x_ = *x;
    float y_ = *y;
    // float z_ = *z;

    *x = x_ * cos_alpha - y_ * sin_alpha;
    *y = x_ * sin_alpha + y_ * cos_alpha;
    // *z = z_;
}


void calcAndPlotIndice(char *frame, float *z_buffer, const char *brightness_ramp, float cos_theta, float sin_theta, float cos_phi, float sin_phi, float cos_alpha_X, float sin_alpha_X, float cos_alpha_Z, float sin_alpha_Z) {
	
	// Offseting the circle along X-axis by R2 so that it sits at the edge of the donut hole
	float x = (RADIUS_2 + RADIUS_1 * cos_theta) * cos_phi;
	float y = RADIUS_1 * sin_theta;
	float z = (RADIUS_2 + RADIUS_1 * cos_theta) * sin_phi;

    // Normal vectors !!!
    // *Skipping one step ahead (Directly sweeping these normal vectors around the donut center hole)*
    float n_x = cos_theta * cos_phi;
    float n_y = sin_theta;
    float n_z = cos_theta * sin_phi;

	rotateX(&x, &y, &z, cos_alpha_X, sin_alpha_X);
    // rotateY(&x, &y, &z, cos_alpha_Y, sin_alpha_Y);
    rotateZ(&x, &y, &z, cos_alpha_Z, sin_alpha_Z);

    // ALSO ROTATING THE NORMAL VECTORS ALONG WITH THE POINTS
    rotateX(&n_x, &n_y, &n_z, cos_alpha_X, sin_alpha_X);
    // rotateY(&n_x, &n_y, &n_z, cos_alpha_Y, sin_alpha_Y);
    rotateZ(&n_x, &n_y, &n_z, cos_alpha_Z, sin_alpha_Z);

	float z_final = z + Z; // z_final = z_original + z_offset
	float inverse_depth = 1 / z_final;

	// SCREEN COORDINATES
	float x_ = 2.0f * x * (FOV * inverse_depth); // Scaling x_ by 2.0 for the Terminal Aspect Ratio problem
	float y_ = y * (FOV * inverse_depth);

	// MAPPING TO TERMINAL
	int X = roundf(WIDTH/2.0f + x_);
	int Y = roundf(HEIGHT/2.0f - y_);

	// 2D boundary checking to protect both borders and newlines
	if ((X >= 0 && X < WIDTH) && (Y >= 0 && Y < HEIGHT)) {

		// FINAL INDEX
	    int final_idx = Y * (WIDTH + 1) + X;
		
		// CHECKING OCCLUSION AND PLOTTING
		if ((inverse_depth > z_buffer[final_idx]) && (final_idx >= 0 && final_idx < (WIDTH + 1) * HEIGHT)) {
			
			// UPDATE BUFFER
			z_buffer[final_idx] = inverse_depth;

            // CALCULATE LUMINANCE
            float luminance = n_y - n_z; // SIMPLIFIED VERSION

            // PLOT THE POINT
            if (luminance <= 0) {
                frame[final_idx] = '.';
            }
            else {
                int brightness_index = ((int)(luminance * 8) > 11) ? 11 : (int)(luminance * 8);
                frame[final_idx] = brightness_ramp[brightness_index];
            }

		}
	}
}


int main(void) {

    constexpr char brightness_ramp[] = ".,-~:;=!*#$@";
	constexpr int frame_size = (WIDTH + 1) * HEIGHT; // True compile-time constant array size
	char frame[frame_size + 1];
	float z_buffer[frame_size]; // For the Occlusion problem

	// Rotation angles
	float alpha_X = 0.0f;
	// float alpha_Y = 0.0f;
	float alpha_Z = 0.0f;

	// Initializing the rotation stuff
	// For X-axis
	float cos_alpha_X = cosf(alpha_X);
	float sin_alpha_X = sinf(alpha_X);
	
	// For Y-axis
	// float cos_alpha_Y = cosf(alpha_Y);
	// float sin_alpha_Y = sinf(alpha_Y);

	// For Z-axis
	float cos_alpha_Z = cosf(alpha_Z);
	float sin_alpha_Z = sinf(alpha_Z);
	

	printf("\x1b[?25l");

	while (true) {
		// Clearing the frame
		memset(frame, ' ', frame_size);
		// Clearing the z_buffer
		memset(z_buffer, 0.0f, sizeof(z_buffer));

		// Write the \0
		frame[frame_size] = '\0';

		// Write the \n
		for(int i = 1; i < HEIGHT + 1; i++) {
			frame[(WIDTH + 1) * i - 1] = '\n';
		}

		for (float phi = 0.0f; phi < 2 * PI; phi += STEP_2) {
			float cos_phi = cosf(phi);
			float sin_phi = sinf(phi);

			for (float theta = 0.0f; theta < 2 * PI; theta += STEP_1) {
				float cos_theta = cosf(theta);
				float sin_theta = sinf(theta);

				calcAndPlotIndice(frame, z_buffer, brightness_ramp, cos_theta, sin_theta, cos_phi, sin_phi, cos_alpha_X, sin_alpha_X, cos_alpha_Z, sin_alpha_Z);
			}
		}

		printf("\x1b[H%s", frame); // Print the entire frame to the terminal

		// UPDATING THE ROTATION STUFF
		alpha_X += 0.05f;
		// alpha_Y += 0.02f;
		alpha_Z += 0.03f;

		cos_alpha_X = cosf(alpha_X);
		sin_alpha_X = sinf(alpha_X);
		// cos_alpha_Y = cosf(alpha_Y);
		// sin_alpha_Y = sinf(alpha_Y);
		cos_alpha_Z = cosf(alpha_Z);
		sin_alpha_Z = sinf(alpha_Z);


		thrd_sleep(&(struct timespec){.tv_nsec = 16000000}, NULL); // 60 FPS

	}

	printf("\x1b[?25h"); // Restores the cursor

	return 0;
}
