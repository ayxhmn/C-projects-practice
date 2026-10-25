#include<stdio.h>
#include<string.h>
#include<math.h>
#include<threads.h>

#define WIDTH 60
#define HEIGHT 20
#define PI 3.14159265f
#define RADIUS 2.0f
#define Z_OFFSET 5.0f
#define FOV 20.0f   // K1 (Scale Factor)
#define STEP 0.02f // 1 deg = approx. 0.02 rad


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


int calcIndex(float theta, float cos_alpha_X, float sin_alpha_X, float cos_alpha_Z, float sin_alpha_Z) {
    float x = RADIUS * cosf(theta);
    float y = RADIUS * sinf(theta);
    float z = 0.0f;

    rotateX(&x, &y, &z, cos_alpha_X, sin_alpha_X);
    // rotateY(&x, &y, &z, cos_alpha_Y, sin_alpha_Y);
    rotateZ(&x, &y, &z, cos_alpha_Z, sin_alpha_Z);

    float z_final = z + Z_OFFSET;
    float scale = FOV / z_final;
    
    // SCREEN COORDINATES
    float x_ = 2.0f * x * scale; // Extra 2.0f for the Terminal aspect ratio problem
    float y_ = y * scale;

    // MAPPING TO TERMINAL
    int X = roundf(WIDTH / 2.0f + x_);
    int Y = roundf(HEIGHT / 2.0f - y_);

    // 2D Boundary Checking to protect both borders and newlines
    if ((X >= 0 && X < WIDTH) && (Y >= 0 && Y < HEIGHT)) {
        return (Y * (WIDTH + 1) + X);
    }
    
    return -1; // Out of bounds indicator
}


int main(void) {

    constexpr int frame_size = (WIDTH + 1) * HEIGHT; // +1 for '\n's
    char frame[frame_size + 1]; // +1 for the '\0'

    // ROTATION STUFF
    float alpha_X = 0.0f;
    // float alpha_Y = 0.0f;
    float alpha_Z = 0.0f;

    float cos_alpha_X = cosf(alpha_X);
    float sin_alpha_X = sinf(alpha_X);
    // float cos_alpha_Y = cosf(alpha_Y);
    // float sin_alpha_Y = sinf(alpha_Y);
    float cos_alpha_Z = cosf(alpha_Z);
    float sin_alpha_Z = sinf(alpha_Z);

    printf("\x1b[?25l"); // Hide the cursor
    
    while (true) {
        
        // Clearing the frame
        memset(frame, ' ', frame_size);
        
        // Write the '\0'
        frame[frame_size] = '\0';

        // Write the '\n's
        for (int i = 1; i < HEIGHT + 1; i++) {
            frame[(WIDTH + 1) * i - 1] = '\n';
        }

        // Write the '#'s and Rotate
        for (float theta = 0.0f; theta < 2 * PI; theta += STEP) {
            int target_idx = calcIndex(theta, cos_alpha_X, sin_alpha_X, cos_alpha_Z, sin_alpha_Z);

            if (target_idx >= 0 && target_idx < (WIDTH + 1) * HEIGHT) {
                if (frame[target_idx] == ' ') {
                    frame[target_idx] = '#';
                }
            }
        }
        
        // Print the entire frame on the terminal
        printf("\x1b[H%s", frame);

        // Updating the rotation stuff
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

    printf("\x1b[?25h"); // Unhide the cursor
    
    return 0;
}
