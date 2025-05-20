#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <string>

float GetRandomFloat(float min, float max)
{
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

unsigned char GetRandomUChar(unsigned char min, unsigned char max)
{
    return min + static_cast<unsigned char>(rand() % (max - min + 1));
}

// Constants
const int screenWidth = 1280;
const int screenHeight = 720;
const int numMenuItems = 3;
const float itemSpacing = 5.0f;
const float rotationSpeed = 0.1f;
const float hoverScale = 1.2f;
const float introAnimationDuration = 2.0f; // 2 second intro animation

// Menu item class
class MenuItem
{
public:
    Model model;
    Vector3 position;
    Vector3 initialPosition; // For animation
    Vector3 targetPosition;
    Vector3 rotation;
    Vector3 targetRotation;
    Vector3 scale;
    Vector3 initialScale; // For animation
    Vector3 targetScale;
    std::string text;
    Color color;
    Color initialColor; // For animation
    Color targetColor;
    bool selected;

    MenuItem(Model m, Vector3 pos, std::string t)
    {
        model = m;
        position = pos;
        // Set initial position below the screen
        initialPosition = (Vector3){pos.x, pos.y - 20.0f, pos.z};
        position = initialPosition; // Start at initial position
        targetPosition = pos;
        rotation = (Vector3){0.0f, 0.0f, 0.0f};
        targetRotation = rotation;
        // Start with zero scale
        initialScale = (Vector3){0.0f, 0.0f, 0.0f};
        scale = initialScale;
        targetScale = (Vector3){1.0f, 1.0f, 1.0f};
        text = t;
        // Start with transparent color
        initialColor = (Color){DARKGRAY.r, DARKGRAY.g, DARKGRAY.b, 0};
        color = initialColor;
        targetColor = DARKGRAY;
        selected = false;
    }

    void Update()
    {
        // Linear interpolation for smooth animations
        position = Vector3Lerp(position, targetPosition, 0.1f);
        rotation = Vector3Lerp(rotation, targetRotation, 0.1f);
        scale = Vector3Lerp(scale, targetScale, 0.1f);

        // Color interpolation
        color.r = (unsigned char)Lerp(color.r, targetColor.r, 0.1f);
        color.g = (unsigned char)Lerp(color.g, targetColor.g, 0.1f);
        color.b = (unsigned char)Lerp(color.b, targetColor.b, 0.1f);
        color.a = (unsigned char)Lerp(color.a, targetColor.a, 0.1f);
    }

    void Draw(Camera3D camera)
    {
        // Draw 3D model with current transforms only
        DrawModelEx(model, position, (Vector3){0.0f, 1.0f, 0.0f}, rotation.y, scale, color);
    }

    void SetSelected(bool isSelected)
    {
        selected = isSelected;
        if (selected)
        {
            targetColor = BLUE;
            targetScale = (Vector3){hoverScale, hoverScale, hoverScale};
            targetRotation.y += 0.5f; // Small additional rotation for feedback
        }
        else
        {
            targetColor = DARKGRAY;
            targetScale = (Vector3){1.0f, 1.0f, 1.0f};
        }
    }

    // For intro animation
    void AnimateIntro(float progress)
    {
        position = Vector3Lerp(initialPosition, targetPosition, progress);
        scale = Vector3Lerp(initialScale, targetScale, progress);
        color.a = (unsigned char)(progress * 255);
    }
};

// Particle effect for selected item
struct Particle
{
    Vector3 position;
    Vector3 velocity;
    Color color;
    float size;
    float lifeTime;
    float maxLifeTime;
};

class ParticleSystem
{
public:
    std::vector<Particle> particles;

    ParticleSystem()
    {
        // Initialize empty
    }

    void AddParticles(Vector3 position, Color color, int count)
    {
        for (int i = 0; i < count; i++)
        {
            Particle p;
            p.position = position;
            p.velocity = (Vector3){
                GetRandomFloat(-0.05f, 0.05f),
                GetRandomFloat(0.01f, 0.05f),
                GetRandomFloat(-0.05f, 0.05f)};
            p.color = color;
            p.size = GetRandomFloat(0.05f, 0.15f);
            p.lifeTime = 0.0f;
            p.maxLifeTime = GetRandomFloat(1.0f, 2.0f);
            particles.push_back(p);
        }
    }

    void Update()
    {
        for (int i = 0; i < particles.size(); i++)
        {
            // Update particle position
            particles[i].position.x += particles[i].velocity.x;
            particles[i].position.y += particles[i].velocity.y;
            particles[i].position.z += particles[i].velocity.z;

            // Update lifetime
            particles[i].lifeTime += GetFrameTime();

            // Fade based on lifetime
            float alpha = 1.0f - (particles[i].lifeTime / particles[i].maxLifeTime);
            particles[i].color.a = (unsigned char)(int)(255 * alpha);

            // Remove dead particles
            if (particles[i].lifeTime >= particles[i].maxLifeTime)
            {
                particles.erase(particles.begin() + i);
                i--;
            }
        }
    }

    void Draw()
    {
        for (auto &p : particles)
        {
            DrawSphere(p.position, p.size, p.color);
        }
    }
};

int main()
{
    // Initialize window
    InitWindow(screenWidth, screenHeight, "Dendy Streaming Box");

    // Intro animation variables
    bool introAnimationComplete = false;
    float introAnimationTimer = 0.0f;

    // Initialize 3D camera
    Camera3D camera = {0};
    // Start camera further back for intro animation
    camera.position = (Vector3){0.0f, 2.0f, 30.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Final camera position
    Vector3 finalCameraPos = (Vector3){0.0f, 2.0f, 15.0f};

    // Target camera position (for camera interpolation)
    Vector3 targetCameraPos = finalCameraPos;

    // Create menu items
    std::vector<MenuItem> menuItems;

    // Generate models for menu items
    Model sphereModel = LoadModelFromMesh(GenMeshSphere(1.2f, 16, 16));

    // Position menu items in a row
    float startX = -((numMenuItems - 1) * itemSpacing) / 2.0f;
    menuItems.push_back(MenuItem(sphereModel, (Vector3){startX, 0.0f, 0.0f}, "Option 1"));
    menuItems.push_back(MenuItem(sphereModel, (Vector3){startX + itemSpacing, 0.0f, 0.0f}, "Option 2"));
    menuItems.push_back(MenuItem(sphereModel, (Vector3){startX + 2 * itemSpacing, 0.0f, 0.0f}, "Option 3"));

    // Background rotation animation
    float backgroundRotation = 0.0f;

    // Particle system
    ParticleSystem particleSystem;

    // Set current selected item
    int currentItem = 0;
    menuItems[currentItem].SetSelected(true);

    // Set target framerate
    SetTargetFPS(60);

    // Initialize audio
    InitAudioDevice();
    Sound selectSound = LoadSound("resources/select.wav"); // You need to provide this file
    Sound moveSound = LoadSound("resources/move.wav");     // You need to provide this file
    Sound introSound = LoadSound("resources/intro.wav");   // Add an intro sound

    // For Xbox-like ambiance, we'll create a pulsating background
    float pulseTimer = 0.0f;
    float pulseScale = 1.0f;

    // Background elements alpha for intro animation
    float backgroundAlpha = 0.0f;

    // Play intro sound at start
    PlaySound(introSound);

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update intro animation timer
        if (!introAnimationComplete)
        {
            introAnimationTimer += GetFrameTime();

            // Calculate animation progress (0.0 to 1.0)
            float progress = introAnimationTimer / introAnimationDuration;

            // Clamp progress to 0.0-1.0
            if (progress > 1.0f)
            {
                progress = 1.0f;
                introAnimationComplete = true;
            }

            // Animate camera
            camera.position = Vector3Lerp((Vector3){0.0f, 2.0f, 30.0f}, finalCameraPos, progress);

            // Animate menu items
            for (int i = 0; i < (int)menuItems.size(); i++)
            {
                // Stagger the animations slightly
                float itemProgress = progress - (0.1f * i);
                if (itemProgress < 0)
                    itemProgress = 0;
                if (itemProgress > 1)
                    itemProgress = 1;

                menuItems[i].AnimateIntro(itemProgress);
            }

            // Animate background alpha
            backgroundAlpha = progress;

            // Add particles during intro animation
            if (GetRandomFloat(0, 1) < 0.1f && progress > 0.5f)
            {
                float randomX = GetRandomFloat(-5.0f, 5.0f);
                float randomY = GetRandomFloat(-3.0f, 3.0f);
                particleSystem.AddParticles((Vector3){randomX, randomY, 0},
                                            (Color){GetRandomUChar(100, 255), GetRandomUChar(100, 255), 255, 255},
                                            5);
            }
        }

        // Regular updates once intro is complete
        if (introAnimationComplete)
        {
            backgroundRotation += 0.005f;
            pulseTimer += GetFrameTime();
            pulseScale = 1.0f + 0.05f * sinf(pulseTimer * 2.0f);

            // Update menu items animation
            for (int i = 0; i < (int)menuItems.size(); i++)
            {
                menuItems[i].Update();

                // Make items always rotate slightly to face the camera
                menuItems[i].targetRotation.y = sinf(backgroundRotation + i * 0.5f) * 0.3f;

                // Apply a slight floating effect
                menuItems[i].targetPosition.y = sinf(GetTime() * 0.5f + i) * 0.2f;
            }

            // Handle keyboard input
            if (IsKeyPressed(KEY_RIGHT))
            {
                menuItems[currentItem].SetSelected(false);
                currentItem = (currentItem + 1) % numMenuItems;
                menuItems[currentItem].SetSelected(true);
                PlaySound(moveSound);

                // Add particles to the newly selected item
                particleSystem.AddParticles(menuItems[currentItem].position, BLUE, 20);

                // Move camera to focus on selected item
                targetCameraPos.x = menuItems[currentItem].position.x * 0.5f;
            }

            if (IsKeyPressed(KEY_LEFT))
            {
                menuItems[currentItem].SetSelected(false);
                currentItem = (currentItem - 1 + numMenuItems) % numMenuItems;
                menuItems[currentItem].SetSelected(true);
                PlaySound(moveSound);

                // Add particles to the newly selected item
                particleSystem.AddParticles(menuItems[currentItem].position, BLUE, 20);

                // Move camera to focus on selected item
                targetCameraPos.x = menuItems[currentItem].position.x * 0.5f;
            }

            if (IsKeyPressed(KEY_ENTER))
            {
                PlaySound(selectSound);

                // Handle menu selection
                if (currentItem == 0)
                {
                    // Play option selected
                    // You would add your game start code here
                }
                else if (currentItem == 1)
                {
                    // Settings option selected
                    // You would add your settings code here
                }
                else if (currentItem == 2)
                {
                    // Exit option selected
                    break;
                }

                // Visual effect for selection - add more particles
                particleSystem.AddParticles(menuItems[currentItem].position, GOLD, 50);
            }

            // Update camera position with smooth interpolation
            camera.position = Vector3Lerp(camera.position, targetCameraPos, 0.05f);
        }

        // Update particles
        particleSystem.Update();

        // Begin drawing
        BeginDrawing();
        ClearBackground(BLACK);

        // Begin 3D mode
        BeginMode3D(camera);

        // Draw a grid for reference
        DrawGrid(20, 1.0f);

        // Draw background elements
        for (int i = 0; i < 50; i++)
        {
            float radius = 20.0f;
            float angle = (float)i / 50.0f * 2.0f * PI + backgroundRotation;
            Vector3 pos = {
                cosf(angle) * radius,
                sinf(angle * 2.0f) * 5.0f - 5.0f,
                sinf(angle) * radius};

            Color color = {
                (unsigned char)(20 + sinf(angle) * 20),
                (unsigned char)(40 + cosf(angle) * 20),
                (unsigned char)(100 + sinf(angle * 0.5f) * 20),
                (unsigned char)(100 * backgroundAlpha)};

            DrawSphere(pos, 0.3f * pulseScale, color);
        }

        // Draw menu items
        for (int i = 0; i < (int)menuItems.size(); i++)
        {
            menuItems[i].Draw(camera);
        }

        // Draw particles
        particleSystem.Draw();

        EndMode3D();

        // Text alpha for intro animation
        float textAlpha = introAnimationComplete ? 1.0f : (introAnimationTimer / introAnimationDuration);
        if (textAlpha > 1.0f)
            textAlpha = 1.0f;

        // Now draw all text in 2D mode with backgrounds
        for (int i = 0; i < (int)menuItems.size(); i++)
        {
            // Project 3D position to 2D screen space
            Vector3 textPos = menuItems[i].position;
            textPos.y += 2.5f; // Position text higher above the object
            Vector2 screenPos = GetWorldToScreen(textPos, camera);

            // Set up text
            const char *text = menuItems[i].text.c_str();
            int fontSize = 35; // Much larger text
            int textWidth = MeasureText(text, fontSize);

            // Animate text from right side
            float textOffset = introAnimationComplete ? 0 : (screenWidth - (introAnimationTimer / introAnimationDuration) * screenWidth);

            // Draw background rectangle for text
            Color bgColor = (i == currentItem) ? DARKBLUE : DARKGRAY;
            bgColor.a = (unsigned char)(255 * textAlpha);

            DrawRectangle(
                (int)(screenPos.x - textWidth / 2 - 10 + textOffset),
                (int)(screenPos.y - 10),
                textWidth + 20,
                fontSize + 10,
                bgColor);

            // Draw border for the background
            Color borderColor = (i == currentItem) ? BLUE : LIGHTGRAY;
            borderColor.a = (unsigned char)(255 * textAlpha);

            DrawRectangleLines(
                (int)(screenPos.x - textWidth / 2 - 10 + textOffset),
                (int)(screenPos.y - 10),
                textWidth + 20,
                fontSize + 10,
                borderColor);

            // Draw text (bright color for visibility)
            Color textColor = (i == currentItem) ? WHITE : LIGHTGRAY;
            textColor.a = (unsigned char)(255 * textAlpha);

            DrawText(
                text,
                (int)(screenPos.x - textWidth / 2 + textOffset),
                (int)(screenPos.y),
                fontSize,
                textColor);
        }

        // UI overlay - fade in
        Color titleColor = BLUE;
        titleColor.a = (unsigned char)(255 * textAlpha);

        Color instructionsColor = GRAY;
        instructionsColor.a = (unsigned char)(255 * textAlpha);

        DrawText("Cinemint", 20, 20, 20, titleColor);
        DrawText("Use LEFT/RIGHT to navigate, ENTER to select", 20, screenHeight - 40, 20, instructionsColor);

        // Draw a glow effect around the selected item
        if (introAnimationComplete)
        {
            Vector2 itemPos = GetWorldToScreen(menuItems[currentItem].position, camera);
            DrawCircleGradient(
                itemPos.x, itemPos.y,
                100,
                Fade(BLUE, 0.0f),
                Fade(BLUE, 0.3f * textAlpha));
        }

        EndDrawing();
    }

    // Cleanup
    for (int i = 0; i < (int)menuItems.size(); i++)
    {
        UnloadModel(menuItems[i].model);
    }

    UnloadSound(selectSound);
    UnloadSound(moveSound);
    UnloadSound(introSound);
    CloseAudioDevice();

    CloseWindow();

    return 0;
}