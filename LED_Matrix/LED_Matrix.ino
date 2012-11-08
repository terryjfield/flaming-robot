/*
  LED_Matrix
  
  Board: Arduino Uno
  
  Purpose: This program drives an 8x8 LED matrix, to scroll a message.
  
  Circuit: The Arduino drives transistors that are responsible for a single row/column of the matrix.
  Turning on a given row/column combination lights up the LED at the intersection of the row/column.
  
 */

// The hardware is composed of an 8x8 matrix of LEDs 
#define NUM_HW_ROWS 8    // Number of hardware (LED) rows
#define NUM_HW_COLUMNS 8 // Number of hardware (LED) columns

// Characters are defined in a 6x7 matrix, a 5x7 matrix with an additional blank column on the left side for 
// inter-character spacing.
#define CHAR_HEIGHT 7 // Height in pixels of a character
#define CHAR_WIDTH 6  // Width in pixels of a character, including inter-character pixel for spacing

// The 8x8 matrix is capable of displaying at most 3 characters (or portions of) at a time, e.g. a 
// complete character in the middle of the matrix taking 6 columns, leaving one column to the left and right
// for the portions of 2 other characters.
#define MAX_CHARS_DISPLAYED 3 // Need to display at most 3 characters (or portions of in the HW matrix)
#define NUM_MSG_COLUMNS CHAR_WIDTH * MAX_CHARS_DISPLAYED // Number of columns in the message buffer matrix

// Definitions of the characters
#define CHAR_DEF_MATRIX_COLUMN_COUNT CHAR_WIDTH + 1 // Number of pixel columns for a character plus 1 for the ASCII character itself
#define CHAR_DEF_MATRIX_ROW_COUNT 27 // Number of characters defined

// The bitmaps for the characters are stored as an array of bytes. Each byte is a bitmask that
// defines the pixels that are turned on in a column. In the example below, the letter 'a' is defined
// as 6 bytes. The first column's value is 0, denoting an inter-character column space where no pixels 
// are lit. For column 1, only the second pixel from the bottom is turned on, and the value representing
// this pixel is 2. For the last column, column 6, 5 pixels from the bottom are turned on, and the value
// for the column representing these pixels is 1 + 2 + 4 + 8 = 15.
// 
//
// B       Column
// i  1  2  3  4  5  6 
// t
//
// 64
// 32       
// 16       *  *  *         
// 8                 *
// 4        *  *  *  *
// 2     *           *
// 1        *  *  *  *
//

// Matrix containing 
byte charDefMatrix[CHAR_DEF_MATRIX_ROW_COUNT][CHAR_DEF_MATRIX_COLUMN_COUNT] 
  = {'a',0,2,21,21,21,15,
     'b',0,255,5,9,9,6,
     'c',0,14,17,17,17,2,
     'd',0,6,9,9,5,255,
     'e',0,14,21,21,21,12,
     'f',0,8,63,72,64,32,
     'g',0,24,37,37,37,62,
     'h',0,255,8,16,16,15,
     'i',0,0,0,47,0,0,
     'j',0,2,1,17,94,0,
     'k',0,127,4,10,17,0,
     'l',0,0,65,127,1,0,
     'm',0,31,16,12,16,15,
     'n',0,31,8,16,16,15,
     'o',0,14,17,17,17,14,
     'p',0,31,20,20,20,8,
     'q',0,8,20,20,12,31,
     'r',0,31,8,16,16,8,
     's',0,9,21,21,21,2,
     't',0,16,126,17,1,2,
     'u',0,31,1,1,2,31,
     'v',0,28,2,1,2,28,
     'w',0,30,1,6,1,30,
     'x',0,17,10,4,10,17,
     'y',0,24,5,5,5,62,
     'z',0,17,19,21,25,17,  
     ' ',0,0,0,0,0,0};


byte row[] = {2,3,4,5,6,7,8,9}; // Arduino pins #s for rows
byte column[] = {A3, 10, 11, 12, 13, A0, A1, A2}; // Arduino pin #s for columns

void setup() 
{
  // Set digital pins for output
  for (int r = 0; r < NUM_HW_ROWS; r++)
  {
    pinMode(row[r], OUTPUT);
  }
  
  for (int c = 0; c < NUM_HW_COLUMNS; c++)
  {
    pinMode(column[c], OUTPUT);
  }
}

void clearMsgMatrix(byte msgMatrix[][NUM_MSG_COLUMNS])
{
  for (int r = 0; r < NUM_HW_ROWS; r++)
  {
    for (int c = 0; c < NUM_MSG_COLUMNS; c++)
    {
      msgMatrix[r][c] = 0;
    }
  }
}

void insertCharInMatrix(char c, byte msgMatrix[][NUM_MSG_COLUMNS], byte columnPtr)
{
  byte powerOfTwo;
  
  int currentColumnPtr = columnPtr;
  
  for (int j = 0; j < CHAR_DEF_MATRIX_ROW_COUNT; j++) // loop looking for a match in the character set
  {
    if (c == charDefMatrix[j][0]) // do we have a match
    { 
      // turn on bits in the column
      for (int col = 1; col < CHAR_DEF_MATRIX_COLUMN_COUNT; col++)
      {
        powerOfTwo = 1;
        for (int row = 0; row < CHAR_HEIGHT; row++)
        {
          msgMatrix[NUM_HW_ROWS - row - 1][currentColumnPtr] = charDefMatrix[j][col] & powerOfTwo;
          powerOfTwo = powerOfTwo * 2;
        } // row
        currentColumnPtr++;
      } // col
      break;
    } // if
  } //for
}

void displayHWbuffer(byte HWmatrix[][NUM_HW_COLUMNS], int columnTime, int interval)
{
  unsigned long currentMillis  = millis();
  unsigned long previousMillis = currentMillis;
  
  while (currentMillis - previousMillis < interval)
  {    
    int currentColumnPtr = 0;
    for (int ledColumn = 0; ledColumn < NUM_HW_COLUMNS; ledColumn++)
    {
      for (int ledRow = 0; ledRow < NUM_HW_ROWS; ledRow++)
      {
        if (HWmatrix[ledRow][currentColumnPtr] != 0)
        {
          digitalWrite(row[ledRow], HIGH);
        }
        else
        {
          digitalWrite(row[ledRow], LOW);
        }
      }
      digitalWrite(column[ledColumn], HIGH); // turn on column
      delay(columnTime);
      digitalWrite(column[ledColumn], LOW);  // turn off column
      currentColumnPtr++;
    }
    currentMillis = millis();
  } // while
}

void fillHWbuffer(byte windowColPtr, byte msgMatrix[][NUM_MSG_COLUMNS], byte HW_Matrix[][NUM_HW_COLUMNS] )
{
  byte workingWindowColPtr = windowColPtr;
  
  for (int col = 0; col < NUM_HW_COLUMNS; col++)
  {
    for (int row = 0; row < NUM_HW_ROWS; row++)
    {
      HW_Matrix[row][col] = msgMatrix[row][workingWindowColPtr];
      
      if (workingWindowColPtr == NUM_MSG_COLUMNS) // wrap around to beginning of matrix if we have reached the end
      {
        workingWindowColPtr = 0;
      }
    } //row
    workingWindowColPtr++;
  } // col
}

void scrollWindow(char msg[])
{  
  byte msgMatrix[NUM_HW_ROWS][NUM_MSG_COLUMNS]; // Matrix contains message, i.e. multiple characters

  byte hwBuffer[NUM_HW_ROWS][NUM_HW_COLUMNS];
  byte tailPtr = 0;    // tail of the message array
  byte windowPtr = 0;  // pointer into message array, leftmost column to display
  byte msgCharPtr = 0; // pointer into message, next character to add to message array
  
  clearMsgMatrix(msgMatrix);
  while (true)
  { 
    fillHWbuffer(windowPtr, msgMatrix, hwBuffer);
    displayHWbuffer(hwBuffer, 2, 60);
    windowPtr++;
    
    if (windowPtr % CHAR_WIDTH == 0)
    {
      if (msg[msgCharPtr] == '\0') // are we at the end of the message string?
      {
        msgCharPtr = 0; // if so, reset to beginning of message string
      }
      
      insertCharInMatrix(msg[msgCharPtr], msgMatrix, tailPtr);
      
      msgCharPtr++;
      
      tailPtr += CHAR_WIDTH;
      if (tailPtr == NUM_MSG_COLUMNS) // at the end of the msgMatrix
      {
        tailPtr = 0;
      }
    }
    
    if (windowPtr == NUM_MSG_COLUMNS)
    {
      windowPtr = 0;
    }
    
  } //while
}

void testMsg()
{
  scrollWindow(" slurp");
}

void loop() {
  testMsg();
}
