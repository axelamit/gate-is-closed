import serial
import pygame

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


blocksize = 100
drawing_scale = 1

size = blocksize * 10 * drawing_scale

screen = pygame.display.set_mode([size, size])
font = pygame.font.Font(pygame.font.get_default_font(), 20)
font2 = pygame.font.Font(pygame.font.get_default_font(), 40)


running = True
while (running):

    for event in pygame.event.get():
        if (event.type == pygame.QUIT): 
            running = False

    screen.fill((0, 0, 0))

    gamestate = get_gamestate()
    
    # Draw map 
    for i in range(gamestate[3]):
        for j in range(gamestate[2]): 
            if (i == 0 or j == 0 or i == gamestate[3]-1 or j == gamestate[2]-1):
                pass
            else:
                # Ground
                n = (hash('{}'.format((j * blocksize * drawing_scale, i * blocksize * drawing_scale))))%12
                tile_file_name = 'images/tile{}.png'.format(n+1)
                # tile_file_name = 'images/tile{}.png'.format(1)
                tile = pygame.image.load(tile_file_name)
                tile = pygame.transform.scale(tile, (int(blocksize * drawing_scale), int(blocksize * drawing_scale)))
                tile.convert()
                screen.blit(tile, (j * blocksize * drawing_scale, i * blocksize * drawing_scale))
                #pygame.draw.rect(screen, (211,211,211), (j * blocksize * drawing_scale, i * blocksize * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))

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
            
            cable_offset = 9

            if (main_x == next_x):
                if (wire_state == 1):
                    cable = pygame.image.load('images/cable_on_vertical.png')
                    cable = pygame.transform.scale(cable, (int(blocksize * drawing_scale ), int((abs(main_y - next_y) + 1) * blocksize * drawing_scale - blocksize*drawing_scale + cable_offset*2)))
                    cable.convert()
                    screen.blit(cable, (main_x * blocksize * drawing_scale, min(main_y, next_y) * blocksize * drawing_scale + blocksize*drawing_scale/2 - cable_offset))
                else:
                    cable = pygame.image.load('images/cable_off_vertical.png')
                    cable = pygame.transform.scale(cable, (int(blocksize * drawing_scale ), int((abs(main_y - next_y) + 1) * blocksize * drawing_scale - blocksize*drawing_scale + cable_offset*2)))
                    cable.convert()
                    screen.blit(cable, (main_x * blocksize * drawing_scale, min(main_y, next_y) * blocksize * drawing_scale + blocksize*drawing_scale/2 - cable_offset))
            else:
                if (wire_state == 1):
                    cable = pygame.image.load('images/cable_on_horizontal.png')
                    cable = pygame.transform.scale(cable, (int((abs(main_x - next_x) + 1) * blocksize * drawing_scale) - blocksize*drawing_scale + cable_offset*2, int(blocksize * drawing_scale)))
                    cable.convert()
                    screen.blit(cable, (min(main_x, next_x) * blocksize * drawing_scale + blocksize*drawing_scale/2 - cable_offset, main_y * blocksize * drawing_scale))
                else:
                    cable = pygame.image.load('images/cable_off_horizontal.png')
                    cable = pygame.transform.scale(cable, (int((abs(main_x - next_x) + 1) * blocksize * drawing_scale) - blocksize*drawing_scale + cable_offset*2, int(blocksize * drawing_scale)))
                    cable.convert()
                    screen.blit(cable, (min(main_x, next_x) * blocksize * drawing_scale + blocksize*drawing_scale/2 - cable_offset, main_y * blocksize * drawing_scale))

            if (next_type == 2):
                tile_spess = pygame.image.load('images/tile_spess.png')
                tile_spess = pygame.transform.scale(tile_spess, (int(blocksize * drawing_scale), int(blocksize * drawing_scale)))
                tile_spess.convert()
                screen.blit(tile_spess, (next_x * blocksize * drawing_scale, next_y * blocksize * drawing_scale))

        if (main_type == 2):
            tile_spess = pygame.image.load('images/tile_spess.png')
            tile_spess = pygame.transform.scale(tile_spess, (int(blocksize * drawing_scale), int(blocksize * drawing_scale)))
            tile_spess.convert()
            screen.blit(tile_spess, (main_x * blocksize * drawing_scale, main_y * blocksize * drawing_scale))

    # Draw map 
    for i in range(gamestate[3]):
        for j in range(gamestate[2]): 
            if (i == 0 or j == 0 or i == gamestate[3]-1 or j == gamestate[2]-1):
                # Wall
                n = (hash('{}'.format((j * blocksize * drawing_scale, i * blocksize * drawing_scale))))%9
                tile_file_name = 'images/wall{}.png'.format(n+1)
                # tile_file_name = 'images/tile{}.png'.format(1)
                tile = pygame.image.load(tile_file_name)
                tile = pygame.transform.scale(tile, (int(blocksize * drawing_scale), int(blocksize * drawing_scale)))
                tile.convert()
                screen.blit(tile, (j * blocksize * drawing_scale, i * blocksize * drawing_scale))
                
                #pygame.draw.rect(screen, (95,90,82), (j * blocksize * drawing_scale, i * blocksize * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))
            else:
                pass

    # Door
    if (gamestate[4]):
        door_closed = pygame.image.load('images/door_closed.png')
        door_closed = pygame.transform.scale(door_closed, (int(blocksize * drawing_scale), int(blocksize * drawing_scale)))
        door_closed.convert()
        screen.blit(door_closed, (gamestate[6] * blocksize * drawing_scale, gamestate[5] * blocksize * drawing_scale))
    else:
        door_open = pygame.image.load('images/door_open.png')
        door_open = pygame.transform.scale(door_open, (int(blocksize * drawing_scale), int(blocksize * drawing_scale)))
        door_open.convert()
        screen.blit(door_open, (gamestate[6] * blocksize * drawing_scale, gamestate[5] * blocksize * drawing_scale))
        #pygame.draw.rect(screen, (255,255,255), (gamestate[6] * blocksize * drawing_scale, gamestate[5] * blocksize * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))


    # Draw boxes
    n_boxes = gamestate[buffer_index]
    buffer_index += 1

    for i in range(n_boxes):
        if (gamestate[buffer_index] == 0):
            # And
            box_and = pygame.image.load('images/box_and.png')
            box_and = pygame.transform.scale(box_and, (int(blocksize * drawing_scale), int(blocksize * drawing_scale)))
            box_and.convert()
            screen.blit(box_and, (gamestate[buffer_index + 1] * drawing_scale, gamestate[buffer_index + 2] * drawing_scale))
        elif (gamestate[buffer_index] == 1):
            # Or
            box_or = pygame.image.load('images/box_or.png')
            box_or = pygame.transform.scale(box_or, (int(blocksize * drawing_scale), int(blocksize * drawing_scale)))
            box_or.convert()
            screen.blit(box_or, (gamestate[buffer_index + 1] * drawing_scale, gamestate[buffer_index + 2] * drawing_scale))
        elif (gamestate[buffer_index] == 2):
            # Not
            pygame.draw.rect(screen, (0,255,255), (gamestate[buffer_index + 1] * drawing_scale, gamestate[buffer_index + 2] * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))
        elif (gamestate[buffer_index] == 3):
            box_and = pygame.image.load('images/box_nand.png')
            box_and = pygame.transform.scale(box_and, (int(blocksize * drawing_scale), int(blocksize * drawing_scale)))
            box_and.convert()
            screen.blit(box_and, (gamestate[buffer_index + 1] * drawing_scale, gamestate[buffer_index + 2] * drawing_scale))
        elif (gamestate[buffer_index] == 4):
            box_and = pygame.image.load('images/box_nor.png')
            box_and = pygame.transform.scale(box_and, (int(blocksize * drawing_scale), int(blocksize * drawing_scale)))
            box_and.convert()
            screen.blit(box_and, (gamestate[buffer_index + 1] * drawing_scale, gamestate[buffer_index + 2] * drawing_scale))
        else:
            pygame.draw.rect(screen, (255,255,0), (gamestate[buffer_index + 1] * drawing_scale, gamestate[buffer_index + 2] * drawing_scale, blocksize * drawing_scale, blocksize * drawing_scale))
        
        buffer_index += 3

    # Draw time
    text = "Time: {}".format(gamestate[buffer_index])
    #text_surface = font.render(text)
    text_surface = font.render(text, True, (255, 255, 255))
    screen.blit(text_surface, dest=(10,10))
    buffer_index += 1

    # Display simulation state
    text = "State: {}/{}".format(gamestate[buffer_index], gamestate[buffer_index+1])
    text_surface = font.render(text, True, (255, 255, 255))
    screen.blit(text_surface, dest=(10,30))
    buffer_index += 2

    #Draw player
    character = pygame.image.load('images/character.png')
    character = pygame.transform.scale(character, (int(blocksize * drawing_scale), int(blocksize * drawing_scale)))
    character.convert()
    screen.blit(character, (gamestate[0] * drawing_scale, gamestate[1] * drawing_scale))

    # Select name
    select_name = gamestate[buffer_index]
    buffer_index += 1
    if (select_name):
        stop = int(size/blocksize)
        for k in range(stop):
            for l in range(stop):
                n = (hash('{}'.format((k+l *k -l))))%9
                tile_file_name = 'images/wall{}.png'.format(n+1)
                wall = pygame.image.load(tile_file_name)
                wall = pygame.transform.scale(wall, (int(blocksize * drawing_scale), int(blocksize * drawing_scale)))
                wall.convert()
                screen.blit(wall, (k * blocksize * drawing_scale, l * blocksize * drawing_scale))

        text = "Name: {}{}{}".format(chr(gamestate[buffer_index]), chr(gamestate[buffer_index + 1]), chr(gamestate[buffer_index + 2]))
        text_surface = font2.render(text, True, (0, 0, 0))
        screen.blit(text_surface, dest=((stop/4)* blocksize* drawing_scale,(stop/4) * blocksize  * drawing_scale))
        buffer_index += 3

    # Display scoreboard
    display_scoreboard = gamestate[buffer_index]
    buffer_index += 1
    if (display_scoreboard):
        stop = int(size/blocksize)
        for k in range(stop):
            for l in range(stop):
                n = (hash('{}'.format((k+l *k -l))))%9
                tile_file_name = 'images/wall{}.png'.format(n+1)
                wall = pygame.image.load(tile_file_name)
                wall = pygame.transform.scale(wall, (int(blocksize * drawing_scale), int(blocksize * drawing_scale)))
                wall.convert()
                screen.blit(wall, (k * blocksize * drawing_scale, l * blocksize * drawing_scale))
        seen = 0

        scoreboard = pygame.image.load('images/scoreboard.png')
        scoreboard = pygame.transform.scale(scoreboard, (int(blocksize * drawing_scale*4), int(blocksize * drawing_scale*6)))
        scoreboard.convert()
        screen.blit(scoreboard, ((stop/3.5)* blocksize* drawing_scale,(stop/4.5) * blocksize  * drawing_scale))

        for i in range(3):
            if (gamestate[buffer_index + i] != 0):
                text = "{}. {}{}{}: {} s".format(seen+1, chr(gamestate[buffer_index + (i+1)*3]), chr(gamestate[buffer_index + (i+1)*3 + 1]), chr(gamestate[buffer_index + (i+1)*3 + 2]), gamestate[buffer_index + i])
                text_surface = font2.render(text, True, (0, 0, 0))
                screen.blit(text_surface, dest=((stop/3)* blocksize* drawing_scale,(stop/3) * blocksize  * drawing_scale + seen*blocksize*1.2))
                seen += 1
        buffer_index += 12
        
    pygame.display.update()

pygame.quit()