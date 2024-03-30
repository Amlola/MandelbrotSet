#include "../inc/mandlebrot.h"



void GetMandelBrotSet(MandelBrot* set)
    {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set");

    sf::Image image;
    image.create(WIDTH, HEIGHT, sf::Color::Black);

    sf::Texture texture = {};
    texture.loadFromImage(image);

    sf::Sprite sprite = {};
    sprite.setTexture(texture);

    sf::Font font;
    font.loadFromFile("fyodor-bold-oblique.ttf");
    sf::Text text("", font, 20);
    text.setFillColor(sf::Color::Blue);
    text.setPosition(25, 5);

    sf::Clock clock = {};

    while (window.isOpen())
        {
        sf::Event event;

        if (window.pollEvent(event))
            {
            if (event.type == sf::Event::Closed) window.close();

            KeyboardStatus(set, event);
            }

        GetFPS(clock, text, set);

        texture.update((sf::Uint8*)set->pixels_array, WIDTH, HEIGHT, 0, 0);
        window.clear();
        window.draw(sprite);
        window.draw(text);
        window.display();
        }
    }


void BuildingImageByNaive(MandelBrot* set)
    {
    const float real_dx = set->scale * D_X;

    for (size_t y = 0; y < HEIGHT; y++)
        {
        float x0 = (-(HALF_WIDTH)            * D_X + set->x_offset) * set->scale;
        float y0 = (((float)y - HALF_HEIGHT) * D_Y + set->y_offset) * set->scale;

        for (size_t x = 0; x < WIDTH; x++, x0 += real_dx)
            {
            float x_n = x0;
            float y_n = y0;
            size_t count = 0;

            while (count++ < MAX_ITERATIONS)
                {
                float x2 = x_n * x_n;
                float y2 = y_n * y_n;
                float xy = x_n * y_n;

                if (x2 + y2 >= MAX_RADIUS) break;

                x_n = x2 - y2 + x0;
                y_n = xy + xy + y0;
                }

            GetPixels(set, y * WIDTH + x, count - 1);
            }
        }
    }


void BuildingImageByArray(MandelBrot* set)
    {
    const float real_dx = set->scale * D_X;

    for (size_t y = 0; y < HEIGHT; y++) 
        {
        float x0 = (-(HALF_WIDTH)            * D_X + set->x_offset) * set->scale;
        float y0 = (((float)y - HALF_HEIGHT) * D_Y + set->y_offset) * set->scale;

        for (size_t x = 0; x < WIDTH; x += VECTOR_SIZE, x0 += VECTOR_SIZE * real_dx) 
            {
            float X0[VECTOR_SIZE] = {x0,               x0 + real_dx, 
                                     x0 + 2 * real_dx, x0 + 3 * real_dx,
                                     x0 + 4 * real_dx, x0 + 5 * real_dx, 
                                     x0 + 6 * real_dx, x0 + 7 * real_dx};

            float X_N[VECTOR_SIZE] = {};
            float Y_N[VECTOR_SIZE] = {}; 

            for (size_t i = 0; i < VECTOR_SIZE; i++)
                {
                X_N[i] = X0[i];
                Y_N[i] = y0;
                }
               
            int count = 0;
            int real_count[VECTOR_SIZE] = {};

            while (count++ < MAX_ITERATIONS)
                {
                float X2[VECTOR_SIZE] = {}; 
                float Y2[VECTOR_SIZE] = {}; 
                float XY[VECTOR_SIZE] = {};

                for (size_t i = 0; i < VECTOR_SIZE; i++)
                    { 
                    X2[i] = X_N[i] * X_N[i];
                    Y2[i] = Y_N[i] * Y_N[i];
                    XY[i] = X_N[i] * Y_N[i];
                    }

                int CMP[VECTOR_SIZE] = {};
                for (size_t i = 0; i < VECTOR_SIZE; i++)
                    {
                    if (X2[i] + Y2[i] <= MAX_RADIUS) 
                        CMP[i] = 1;
                    }

                int mask = 0;
                for (size_t i = 0; i < VECTOR_SIZE; i++) 
                    mask |= (CMP[i] << i);

                if (!mask) break;

                for (size_t i = 0; i < VECTOR_SIZE; i++)
                    { 
                    real_count[i] += CMP[i];
                    X_N[i] = X2[i] - Y2[i] + X0[i];
                    Y_N[i] = XY[i] + XY[i] + y0;
                    }
                }

                for (size_t i = 0; i < VECTOR_SIZE; i++) 
                    GetPixels(set, y * WIDTH + x + i, real_count[i]);
            }
        }
    }


void BuildingImageBySimd(MandelBrot* set) 
    {
    float real_dx = D_X * set->scale;

    __m256 MaxRadius = _mm256_set1_ps(MAX_RADIUS);

    __m256 DX = _mm256_set1_ps(real_dx);                                
    __m256 MUL_OFFSET = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);
    DX = _mm256_mul_ps(DX, MUL_OFFSET);

    for (size_t y = 0; y < HEIGHT; y++)
        {
        float x_0 = (-(HALF_WIDTH)            * D_X + set->x_offset) * set->scale;
        float y_0 = (((float)y - HALF_HEIGHT) * D_Y + set->y_offset) * set->scale;
        
        for (size_t x = 0; x < WIDTH; x += VECTOR_SIZE, x_0 += VECTOR_SIZE * real_dx)
            {
            __m256 X0 = _mm256_set1_ps(x_0);
            X0 = _mm256_add_ps(X0, DX);

            __m256 Y0 = _mm256_set1_ps(y_0);

            __m256 X_N = X0;
            __m256 Y_N = Y0;

            int count = 0;
            __m256i real_count = _mm256_setzero_si256();

            while (count++ < MAX_ITERATIONS)
                {
                __m256 X2 = _mm256_mul_ps(X_N, X_N);
                __m256 Y2 = _mm256_mul_ps(Y_N, Y_N);
                __m256 XY = _mm256_mul_ps(X_N, Y_N);

                __m256 R2 = _mm256_add_ps(X2, Y2);

                __m256 res = _mm256_cmp_ps(R2, MaxRadius, _CMP_LE_OS);

                if (!_mm256_movemask_ps(res)) break;

                __m256i temp = _mm256_castps_si256(res);
                temp = _mm256_srli_epi32(temp, 31);
                real_count = _mm256_add_epi32(real_count, temp);

                X_N = _mm256_sub_ps(X2, Y2);
                X_N = _mm256_add_ps(X_N, X0);

                Y_N = _mm256_add_ps(XY, XY);
                Y_N = _mm256_add_ps(Y_N, Y0);
                }

            uint32_t* counts = (uint32_t*)(&real_count);

            for (size_t i = 0; i < VECTOR_SIZE; i++)
                GetPixels(set, y * WIDTH + x + i, counts[i]); 
        } 
    }
}


void ChooseRunTest(MandelBrot* set) 
    {
    switch (set->mode) 
        {
        case BY_PIXELS: 
            {
            RUN_TEST(BuildingImageByNaive);  
            break;
            }

        case BY_VECTOR: 
            {
            RUN_TEST(BuildingImageByArray); 
            break;
            }

        case BY_SIMD:
            {   
            RUN_TEST(BuildingImageBySimd);   
            break;
            }

        default:        
            break;
        }
    }


inline void GetPixels(MandelBrot* set,  size_t index, size_t count)
    {
    if (count != MAX_ITERATIONS)
        {
        count *= 100;
        set->pixels_array[index] = 0xFF | count << 24 | count << 16 | count << 8;
        }

    else
        set->pixels_array[index] = 0xFF000000;
    }


inline int KeyboardStatus(MandelBrot* set, sf::Event &event) 
    {
    if (event.type == sf::Event::KeyPressed)
        {
        switch (event.key.code)
            {            
            case sf::Keyboard::Key::Right:  set->x_offset   += 0.05f; break;

            case sf::Keyboard::Key::Left:   set->x_offset   -= 0.05f; break;

            case sf::Keyboard::Key::Up:     set->y_offset   -= 0.05f; break;

            case sf::Keyboard::Key::Down:   set->y_offset   += 0.05f; break;

            case sf::Keyboard::Key::Hyphen: set->scale      *= 1.25f; break;

            case sf::Keyboard::Key::Equal:  set->scale      *= 0.8f;  break;

            default:                                                  break;
            }
        }
    return 0;
    }


inline void GetFPS(sf::Clock &clock, sf::Text &text, MandelBrot* set) 
    {
    switch (set->mode)
        {
        case BY_PIXELS: 
            {
            clock.restart();
            BuildingImageByNaive(set);  
            break;
            }

        case BY_VECTOR:
            {
            clock.restart();
            BuildingImageByArray(set); 
            break;
            }

        case BY_SIMD:
            { 
            clock.restart(); 
            BuildingImageBySimd(set);   
            break;
            }
                
        default:                                     
            break;
        }

    sf::Time elapsed_time = clock.getElapsedTime();

    char buffer[16] = {};

    sprintf(buffer, "FPS: %.2f", 1.f / elapsed_time.asSeconds());

    text.setString(buffer);
    }


void MandelBrotInit(MandelBrot* set) 
    {
    set->scale      = 3.0f;
    set->x_offset   = -0.1f;
    set->y_offset   = 0.f;
    set->pixels_array = (uint32_t*)calloc(HEIGHT * WIDTH, sizeof(uint32_t));
    }