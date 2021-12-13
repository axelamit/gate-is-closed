import serial
import pygame
import time

pygame.init()

ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
ser.reset_input_buffer()

def read_int(): 
    a = ""
    for i in range(4):
        while True:
            if (ser.in_waiting > 0):
                n = ser.read()
                try:
                    a += str(int(n))
                except:
                    pass
                
                break
    try: 
        b = int(a)
    except:
        return read_int()

    return int(a) 

def get_gamestate():
    n = 0
    while True:
        if (ser.in_waiting > 0):
            n = ser.read()
            if (n == b's'):
                break
    
    n = read_int()

    gamestate = []
    for i in range(n): 
        gamestate.append(read_int())

    return gamestate

screen = pygame.display.set_mode([500, 500])
font = pygame.font.Font(pygame.font.get_default_font(), 20)
font2 = pygame.font.Font(pygame.font.get_default_font(), 40)

blocksize = 100
drawing_scale = 0.5

running = True
while (running):

    for event in pygame.event.get():
        if (event.type == pygame.QUIT): 
            running = False

    screen.fill((255, 255, 255))

    gamestate = get_gamestate()
    
    # Draw map 
    for i in range(gamestate[3]):
        for j in range(gamestate[2]): 
            if (i == 0 or j == 0 or i == gamestate[3]-1 or j == gamestate[2]-1):
                # Wall
                pygame.draw.rect(screen, (211,211,211), (j * blocksize * drawing_scale, i * blocksize * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))
            else:
                # Ground
                pygame.draw.rect(screen, (255,255,255), (j * blocksize * drawing_scale, i * blocksize * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))
            

    # Door
    if (gamestate[4]):
        pygame.draw.rect(screen, (101,67,33), (gamestate[6] * blocksize * drawing_scale, gamestate[5] * blocksize * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))
    else:
        pygame.draw.rect(screen, (255,255,255), (gamestate[6] * blocksize * drawing_scale, gamestate[5] * blocksize * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))

    buffer_index = 7

    # Draw wires
    n_connectors = gamestate[buffer_index]
    buffer_index += 1

    for i in range(n_connectors):
        n_connections = gamestate[buffer_index]
        buffer_index += 1

        main_type = gamestate[buffer_index]
        main_x = gamestate[buffer_index + 1]
        main_y = gamestate[buffer_index + 2]
        buffer_index += 3

        for j in range(n_connections):
            next_type = gamestate[buffer_index]
            next_x = gamestate[buffer_index + 1]
            next_y = gamestate[buffer_index + 2]
            wire_state = gamestate[buffer_index + 3]
            buffer_index += 4

            if (main_x == next_x):
                if (wire_state == 1):
                    pygame.draw.rect(screen, (255,255,0), (main_x * blocksize * drawing_scale + blocksize * drawing_scale / 4, min(main_y, next_y) * blocksize * drawing_scale, blocksize * drawing_scale / 2, (abs(main_y - next_y) + 1) * blocksize * drawing_scale))
                else:
                    pygame.draw.rect(screen, (224,224,224), (main_x * blocksize * drawing_scale + blocksize * drawing_scale / 4, min(main_y, next_y) * blocksize * drawing_scale, blocksize * drawing_scale / 2, (abs(main_y - next_y) + 1) * blocksize * drawing_scale)) 
            else:
                if (wire_state == 1):
                    pygame.draw.rect(screen, (255,255,0), (min(main_x, next_x) * blocksize * drawing_scale, main_y * blocksize * drawing_scale + blocksize * drawing_scale / 4, (abs(main_x - next_x) + 1) * blocksize * drawing_scale, blocksize * drawing_scale / 2))
                else:
                    pygame.draw.rect(screen, (224,224,224), (min(main_x, next_x) * blocksize * drawing_scale, main_y * blocksize * drawing_scale + blocksize * drawing_scale / 4, (abs(main_x - next_x) + 1) * blocksize * drawing_scale, blocksize * drawing_scale / 2))

            if (next_type == 2):
                pygame.draw.rect(screen, (0,0,0), (next_x * blocksize * drawing_scale, next_y * blocksize * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))
        
        if (main_type == 2):
            pygame.draw.rect(screen, (0,0,0), (main_x * blocksize * drawing_scale, main_y * blocksize * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))

    # Draw boxes
    n_boxes = gamestate[buffer_index]
    buffer_index += 1

    for i in range(n_boxes):
        if (gamestate[buffer_index] == 0):
            # And
            pygame.draw.rect(screen, (0,255,0), (gamestate[buffer_index + 1] * drawing_scale, gamestate[buffer_index + 2] * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))
        elif (gamestate[buffer_index] == 1):
            # Or
            pygame.draw.rect(screen, (0,0,255), (gamestate[buffer_index + 1] * drawing_scale, gamestate[buffer_index + 2] * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))
        elif (gamestate[buffer_index] == 2):
            # Not
            pygame.draw.rect(screen, (0,255,255), (gamestate[buffer_index + 1] * drawing_scale, gamestate[buffer_index + 2] * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))
        else:
            pygame.draw.rect(screen, (255,255,0), (gamestate[buffer_index + 1] * drawing_scale, gamestate[buffer_index + 2] * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))
        
        buffer_index += 3

    # Draw time
    text = "Time: {}".format(gamestate[buffer_index])
    #text_surface = font.render(text)
    text_surface = font.render(text, True, (0, 0, 0))
    screen.blit(text_surface, dest=(10,10))
    buffer_index += 1

    #Draw player
    pygame.draw.rect(screen, (255,0,0), (gamestate[0] * drawing_scale, gamestate[1] * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))

    # Select name
    select_name = gamestate[buffer_index]
    buffer_index += 1
    if (select_name):
        screen.fill((255, 255, 255))
        text = "Name: {}{}{}".format(chr(gamestate[buffer_index]), chr(gamestate[buffer_index + 1]), chr(gamestate[buffer_index + 2]))
        text_surface = font2.render(text, True, (0, 0, 0))
        screen.blit(text_surface, dest=(100,100))
        buffer_index += 3

    # Display scoreboard
    display_scoreboard = gamestate[buffer_index]
    buffer_index += 1
    if (display_scoreboard):
        screen.fill((255, 100, 165))
        seen = 0
        for i in range(3):
            if (gamestate[buffer_index + i] != 0):
                text = "{}{}{}: {}".format(chr(gamestate[buffer_index + (i+1)*3]), chr(gamestate[buffer_index + (i+1)*3 + 1]), chr(gamestate[buffer_index + (i+1)*3 + 2]), gamestate[buffer_index + i])
                text_surface = font2.render(text, True, (0, 0, 0))
                screen.blit(text_surface, dest=(100,100 + seen*30))
                seen += 1
        buffer_index += 12
        
    pygame.display.update()

pygame.quit()