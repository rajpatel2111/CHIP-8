
// CHIP8 Emulator

package;

// Import
import openfl.display.Sprite;

import haxe.io.Bytes;
import haxe.io.BytesData;

import Util.cfor;

// Typedef
// typedef U8 = Unsigned_char__;
typedef U8 = Bytes;

class Main extends Sprite {
	public function new() {
		super();
	}
	
	// Display size
	var SCREEN_WIDTH:Int = 64;
	var SCREEN_HEIGHT:Int = 32;
	
	var chip:Chip8 = new Chip8();
	var modifier:Int = 10;
	
	// Window size
	var display_width:Int = SCREEN_WIDTH * modifier;
	var display_height:Int = SCREEN_HEIGHT * modifier;
	
	function display():Void;
	function reshape_window(w:GLsizei, h:GLsizei):Void;
	function keyboardUp(key:Unsigned_char__, x:Int, y:Int):Void;
	function keyboardDown(key:Unsigned_char__, x:Int, y:Int):Void;
	
	// Use new drawing method
	var DRAWWITHTEXTURE:Bool;
	var screenData:U8 = [SCREEN_HEIGHT][SCREEN_WIDTH][3];
	function setupTexture():Void;
	
	public static function main(argc:Int, argv:String) {
		
		if (argc < 2) {
			printf("Usage: chip filename\n\n");
			return 1;
		}
		
		// Load game
		if (!chip.loadApplication(argv[1])) {
			return 1;
		}
		
		// Setup OpenGL
		glutInit(argc, argv);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
		
		glutInitWindowSize(display_width, display_height);
		glutInitWindowPosition(320, 320);
		glutCreateWindow("CHIP8");
		
		glutDisplayFunc(display);
		glutIdleFunc(display);
		glutReshapeFunc(reshape_window);
		glutKeyboardFunc(keyboardDown);
		glutKeyboardUpFunc(keyboardUp);
		
		#if DRAWWITHTEXTURE
			setupTexture();
		#end
		
		glutMainLoop();
		
		/*
		// Set up render system and register input callbacks
		setupGraphics();
		setupInput();
		
		// Initialize the Chip8 system and load the game into the memory  
		chip.initialize();
		chip.loadApplication("pong");
		
		// Emulation loop
		for(;;) {
			// Emulate one cycle
			chip.emulateCycle();
			
			// If the draw flag is set, update the screen
			if(chip.drawFlag) {
				drawGraphics();
			}
			
			// Store key press state (Press and Release)
			chip.setKeys();
		}
		*/
		
		return 0;
	}

	// Setup Texture
	function setupTexture():Void {
		// Clear screen
		cfor (var y = 0, y < SCREEN_HEIGHT, ++y, {
			cfor (var x = 0, x < SCREEN_WIDTH, ++x, {
				screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;
			});
		});
		
		// Create a texture
		glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, cast (screenDate, GLvoid));
		
		// Set up the texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		
		// Enable textures
		glEnable(GL_TEXTURE_2D);
	}

	function updateTexture(chip:Chip8):Void {
		// Update pixels
		cfor (var y = 0, y < 32, ++y, {
			cfor (var x = 0, x < 64, ++x, {
				if (chip.gfx[(y * 64) + x] == 0) {
					screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;	// Disabled
				} else {
					screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 255;	// Enabled
				}
			});
		});
		
		// Update texture
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, cast(screenData, GLVoid));

		glBegin(GL_QUADS);
			glTexCoord2d(0.0, 0.0);		glVertex2d(0.0, 0.0);
			glTexCoord2d(1.0, 0.0);		glVertex2d(display_width, 0.0);
			glTexCoord2d(1.0, 1.0);		glVertex2d(display_width, display_height);
			glTexCoord2d(0.0, 1.0);		glVertex2d(0.0, display_height);
		glEnd();
	}

	function drawPixel(x:Int, y:Int):Void {
		glBegin(GL_QUADS);
			glVertex3f((x * modifier) + 0.0, (y * modifier) + 0.0, 0.0);
			glVertex3f((x * modifier) + 0.0, (y * modifier) + modifier, 0.0);
			glVertex3f((x * modifier) + modifier, (y * modifier) + modifier, 0.0);
			glVertex3f((x * modifier) + modifier, (y * modifier) + 0.0, 0.0);
		glEnd();
	}

	function updateQuads(chip:Chip8):Void {
		// Draw
		cfor (var y = 0, y < 32, ++y, {
			cfor (var x = 0, x < 64, ++x, {
				if (chip.gfx[(y * 64) + x] == 0) {
					glColor3f(0.0, 0.0, 0.0);
				} else {
					glColor3f(1.0, 1.0, 1.0);
				}
				drawPixel(x, y);
			});
		});
	}

	function display():Void {
		chip.emulateCycle();
		
		if (chip.drawFlag) {
			// Clear framebuffer
			glClear(GL_COLOR_BUFFER_BIT);
			
			#if DRAWWITHTEXTURE
				updateTexture(chip);
			#else
				updateQuads(chip);
			#end
			
			// Swap buffers
			glutSwapBuffers();
			
			// Processed frame
			chip.drawFlag = false;
		}
	}

	function reshape_window(w:GLsizei, h:GLsizei):Void {
		glClearColor(0.0, 0.0, 0.5, 0.0);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, w, h, 0);
		glMatrixMode(GL_MODELVIEW);
		glViewport(0, 0, w, h);
		
		// Resize quad
		display_width = w;
		display_height = h;
	}

	function keyboardDown(key:Unsigned_char__, x:Int, y:Int):Void {
		// Esc
		if (key == 27) {
			exit(0);
		}

		if (key == '1') {
			chip.key[0x1] = 1;
		} else if (key == '2') {
			chip.key[0x2] = 1;
		} else if (key == '3') {
			chip.key[0x3] = 1;
		} else if (key == '4') {
			chip.key[0xC] = 1;
		} else if (key == 'q') {
			chip.key[0x4] = 1;
		} else if (key == 'w') {
			chip.key[0x5] = 1;
		} else if (key == 'e') {
			chip.key[0x6] = 1;
		} else if (key == 'r') {
			chip.key[0xD] = 1;
		} else if (key == 'a') {
			chip.key[0x7] = 1;
		} else if (key == 's') {
			chip.key[0x8] = 1;
		} else if (key == 'd') {
			chip.key[0x9] = 1;
		} else if (key == 'f') {
			chip.key[0xE] = 1;
		} else if (key == 'z') {
			chip.key[0xA] = 1;
		} else if (key == 'x') {
			chip.key[0x0] = 1;
		} else if (key == 'c') {
			chip.key[0xB] = 1;
		} else if (key == 'v') {
			chip.key[0xF] = 1;
		}
	}

	function keyboardUp(key:Unsigned_char__, x:Int, y:Int):Void {
		if (key == '1') {
			chip.key[0x1] = 0;
		} else if (key == '2') {
			chip.key[0x2] = 0;
		} else if (key == '3') {
			chip.key[0x3] = 0;
		} else if (key == '4') {
			chip.key[0xC] = 0;
		} else if (key == 'q') {
			chip.key[0x4] = 0;
		} else if (key == 'w') {
			chip.key[0x5] = 0;
		} else if (key == 'e') {
			chip.key[0x6] = 0;
		} else if (key == 'r') {
			chip.key[0xD] = 0;
		} else if (key == 'a') {
			chip.key[0x7] = 0;
		} else if (key == 's') {
			chip.key[0x8] = 0;
		} else if (key == 'd') {
			chip.key[0x9] = 0;
		} else if (key == 'f') {
			chip.key[0xE] = 0;
		} else if (key == 'z') {
			chip.key[0xA] = 0;
		} else if (key == 'x') {
			chip.key[0x0] = 0;
		} else if (key == 'c') {
			chip.key[0xB] = 0;
		} else if (key == 'v') {
			chip.key[0xF] = 0;
		}
	}

}
