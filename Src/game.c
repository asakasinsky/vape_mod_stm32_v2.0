#include "ssd1306.h"
 #include <stdbool.h>

int resolution[2] = {128, 64}, ball[2] = {20, 64};
const int PIXEL_SIZE = 8, WALL_WIDTH = 4, PADDLE_WIDTH = 4, BALL_SIZE = 4, SPEED = 3;
int playerScore = 0, aiScore = 0, playerPos = 0, aiPos = 0;
char ballDirectionHori = 'R', ballDirectionVerti = 'S';
bool inProgress = true;
char playerScore2[5];
char aiScore2[5];


void drawScore() {
  // draw AI and player scores
	
	
 	ssd1306_SetCursor(45, 0);
	sprintf(playerScore2,"%d*",playerScore);
  ssd1306_WriteString(playerScore2,Font_11x18,White);
	
  ssd1306_SetCursor(75, 0);
	sprintf(aiScore2,"%d*",aiScore);
  ssd1306_WriteString(aiScore2,Font_11x18,White);
}

void eraseScore() {
  // erase AI and player scores
 	ssd1306_SetCursor(45, 0);
	sprintf(playerScore2,"%d*",playerScore);
  ssd1306_WriteString(playerScore2,Font_11x18,Black);
	
  ssd1306_SetCursor(75, 0);
	sprintf(aiScore2,"%d*",aiScore);
  ssd1306_WriteString(aiScore2,Font_11x18,Black);
}
void drawPixel_OLED(int posX, int posY, int dimensions) {
  // draw group of pixels
  for (int x = 0; x < dimensions; ++x) {
    for (int y = 0; y < dimensions; ++y) {
      ssd1306_DrawPixel((posX + x), (posY + y), White);
    }
  }
}


void drawNet() {
  for (int i = 0; i < (resolution[1] / WALL_WIDTH); ++i) {
    drawPixel_OLED(((resolution[0] / 2) - 1), i * (WALL_WIDTH) + (WALL_WIDTH * i), WALL_WIDTH);
  }
}



void erasePixel_OLED(int posX, int posY, int dimensions) {
  // erase group of pixels
  for (int x = 0; x < dimensions; ++x) {
    for (int y = 0; y < dimensions; ++y) {
      ssd1306_DrawPixel((posX + x), (posY + y), Black);
    }
  }
}

void erasePlayerPaddle(int row) {
  drawPixel_OLED(0, row - (PADDLE_WIDTH * 2), PADDLE_WIDTH);
  drawPixel_OLED(0, row - PADDLE_WIDTH, PADDLE_WIDTH);
  drawPixel_OLED(0, row, PADDLE_WIDTH);
  drawPixel_OLED(0, row + PADDLE_WIDTH, PADDLE_WIDTH);
  drawPixel_OLED(0, row + (PADDLE_WIDTH + 2), PADDLE_WIDTH);
}

void drawPlayerPaddle(int row) {
  drawPixel_OLED(0, row - (PADDLE_WIDTH * 2), PADDLE_WIDTH);
  drawPixel_OLED(0, row - PADDLE_WIDTH, PADDLE_WIDTH);
  drawPixel_OLED(0, row, PADDLE_WIDTH);
  drawPixel_OLED(0, row + PADDLE_WIDTH, PADDLE_WIDTH);
  drawPixel_OLED(0, row + (PADDLE_WIDTH + 2), PADDLE_WIDTH);
}

void drawAiPaddle_OLED(int row) {
  int column = resolution[0] - PADDLE_WIDTH;
  drawPixel_OLED(column, row - (PADDLE_WIDTH * 2), PADDLE_WIDTH);
  drawPixel_OLED(column, row - PADDLE_WIDTH, PADDLE_WIDTH);
  drawPixel_OLED(column, row, PADDLE_WIDTH);
  drawPixel_OLED(column, row + PADDLE_WIDTH, PADDLE_WIDTH);
  drawPixel_OLED(column, row + (PADDLE_WIDTH * 2), PADDLE_WIDTH);
}

void eraseAiPaddle_OLED(int row) {
  int column = resolution[0] - PADDLE_WIDTH;
  erasePixel_OLED(column, row - (PADDLE_WIDTH * 2), PADDLE_WIDTH);
  erasePixel_OLED(column, row - PADDLE_WIDTH, PADDLE_WIDTH);
  erasePixel_OLED(column, row, PADDLE_WIDTH);
  erasePixel_OLED(column, row + PADDLE_WIDTH, PADDLE_WIDTH);
  erasePixel_OLED(column, row + (PADDLE_WIDTH * 2), PADDLE_WIDTH);
}

void drawBall_OLED(int x, int y) {
  SSD1306_DrawCircle(x, y, BALL_SIZE, White);
}

void eraseBall_OLED(int x, int y) {
  SSD1306_DrawCircle(x, y, BALL_SIZE, Black);
}


void moveAi() {
  // move the AI paddle
  eraseAiPaddle_OLED(aiPos);
  if (ball[1] > aiPos) {
    ++aiPos;
  }
  else if (ball[1] < aiPos) {
    --aiPos;
  }
  drawAiPaddle_OLED(aiPos);
}




void Game() {
  if (aiScore > 9 || playerScore > 9) {
    // check game state
    inProgress = false;
  }

  if (inProgress) {
    eraseScore();
    eraseBall_OLED(ball[0], ball[1]);

    if (ballDirectionVerti == 'U') {
      // move ball up diagonally
      ball[1] = ball[1] - SPEED;
    }

    if (ballDirectionVerti == 'D') {
      // move ball down diagonally
      ball[1] = ball[1] + SPEED;
    }

    if (ball[1] <= 0) { // bounce the ball off the top ballDirectionVerti = 'D'; } if (ball[1] >= resolution[1]) {
      // bounce the ball off the bottom
      ballDirectionVerti = 'U';
    }

    if (ballDirectionHori == 'R') {
      ball[0] = ball[0] + SPEED; // move ball
      if (ball[0] >= (resolution[0] - 6)) {
        // ball is at the AI edge of the screen
        if ((aiPos + 12) >= ball[1] && (aiPos - 12) <= ball[1]) { // ball hits AI paddle if (ball[1] > (aiPos + 4)) {
            // deflect ball down
            ballDirectionVerti = 'D';
          }
          else if (ball[1] < (aiPos - 4)) {
            // deflect ball up
            ballDirectionVerti = 'U';
          }
          else {
            // deflect ball straight
            ballDirectionVerti = 'S';
          }
          // change ball direction
          ballDirectionHori = 'L';
        }
        else {
          // GOAL!
          ball[0] = 6; // move ball to other side of screen
          ballDirectionVerti = 'S'; // reset ball to straight travel
          ball[1] = resolution[1] / 2; // move ball to middle of screen
          ++playerScore; // increase player score
        }
      }
    }

    if (ballDirectionHori == 'L') {
      ball[0] = ball[0] - SPEED; // move ball
      if (ball[0] <= 6) { // ball is at the player edge of the screen if ((playerPos + 12) >= ball[1] && (playerPos - 12) <= ball[1]) { // ball hits player paddle if (ball[1] > (playerPos + 4)) {
            // deflect ball down
            ballDirectionVerti = 'D';
          }
          else if (ball[1] < (playerPos - 4)) { // deflect ball up ballDirectionVerti = 'U'; } else { // deflect ball straight ballDirectionVerti = 'S'; } // change ball direction ballDirectionHori = 'R'; } else { ball[0] = resolution[0] - 6; // move ball to other side of screen ballDirectionVerti = 'S'; // reset ball to straight travel ball[1] = resolution[1] / 2; // move ball to middle of screen ++aiScore; // increase AI score } } } drawBall(ball[0], ball[1]); erasePlayerPaddle(playerPos); playerPos = analogRead(A2); // read player potentiometer playerPos = map(playerPos, 0, 1023, 8, 54); // convert value from 0 - 1023 to 8 - 54 drawPlayerPaddle(playerPos); moveAi(); drawNet(); drawScore(); } else { // somebody has won display.clearDisplay(); display.setTextSize(4); display.setTextColor(WHITE); display.setCursor(0, 0); // figure out who if (aiScore > playerScore) {
      ssd1306_WriteString("YOU  LOSE!",Font_11x18,White);
    }
    else if (playerScore > aiScore) {
      ssd1306_WriteString("YOU  WIN!",Font_11x18,White);
    }
  }

  ssd1306_UpdateScreen();
}


















