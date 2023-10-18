
#include <iostream>
#include <cstdio>
#include <string>
#include <stdarg.h>
#include <queue>
#include <utility>
#include <regex>
#include <map>

#include "queue.hpp"
#include "main.h"
#include "local.hpp"
#include "usbd_cdc_if.h"

using namespace std;


//bool trace_is_on = (DBGMCU->CR & 0b111) > 0;
bool trace_is_on = false;
#ifndef PC
outbuf ob;
streambuf *sb = cout.rdbuf(&ob);
Logger logger;

template<typename T>
void log(T content);


void uprintf(const char* format,...) {
  va_list args;
  va_start(args, format);

  va_list args2;
  va_copy(args2, args);
  char buf[vsnprintf(NULL, 0, format, args) + 1];

  vsnprintf(buf, sizeof buf, format, args2);
  if (trace_is_on) {
    char * c; 
    for (c = buf; *c != '\0'; c++) {
        ITM_SendChar(*c);
    }
    CDC_TransmitString(buf);
  } else {
    //uint8_t result = CDC_TransmitString(buf);
    CDC_TransmitString(buf);
  }

  /*
  // retry once?
  if (result == USBD_BUSY) {
    HAL_Delay(1000);
    CDC_TransmitString(buf);
  }
  */

  va_end(args);
  va_end(args2);

}

int _write(int file, char *ptr, int len) {
    int DataIdx;
    CDC_TransmitString(ptr);
    if (trace_is_on) {
        for (DataIdx = 0; DataIdx < len; DataIdx++) {
            //__io_putchar(*ptr++);
            ITM_SendChar(*ptr++);
        }
    }
    return len;
}
#endif




// list from https://stackoverflow.com/questions/24281603/c-underline-output
// for bold colors, just change the 0 after the [ to a 1
// for underlined colors, just change the 0 after the [ to a 4
string colorText(string str) {

    /*
    // ignore warning about using +17 feature
    //  warning: structured bindings only available with '-std=c++17' or '-std=gnu++17'
    for (const auto& [key, val] : colorMap) {
      //s = std::regex_replace(s, std::regex("#" + std::to_string(id)), value);
      cout << " colorMap[" << colorMap[key] << key << colorMap["reset"] << "] = " << val << colorMap["reset"] << "\n";
    }
    */

    // https://stackoverflow.com/questions/58566267/c-regex-replace-with-a-callback-function
    for (const auto& [key, val] : colorMap) {
        string tag = '<' + key + '>';
        str = regex_replace(str, regex(tag), val);
        //cout << "replace tag " << tag << "\r\n";
    }

    return str;
}

bool memSizeCheck(int n) {
    //uprintf("testing with %d kb\r\n", n);
    int* p = (int *) malloc(n * 1024);
    bool check = (p != NULL);
    free(p);
    //cout << " n=" << n << " kb " << " p= " << p << " check = " << boolalpha << check << "\n";
    return check;
}

int getMemSizeInKB() {
    int low  = 0;
    int high = 100;
    int delta = high;

    // see https://stackoverflow.com/questions/71626597/what-are-the-various-ways-to-disable-and-re-enable-interrupts-in-stm32-microcont
     uint32_t prim = __get_PRIMASK(); // store interrupt status
     __disable_irq(); 

    while (delta > 2) {
        int mid = (low + high) / 2;
        bool check = memSizeCheck(mid);
        if (check) {
            low = mid;
        } else {
            high = mid;
        }
        delta = high - low;
        //cout << " d = " << delta << " space for " << mid << " kb? : " << boolalpha << check << "\n";
    }
    // restore interrupts
    if (!prim) {
        __enable_irq();
    }
    int size = low;
    //cout << " size = " << size << " kB\n";
    return size;
}


int getMemSizeInKBSlow() {
    vector<int*> mem_list;
    int *p;
    bool check;

    mem_list.resize(100); // assume 100kB is upper limit
    mem_list.clear();

    do {
        p = (int *) malloc(1024);
        check = (p != NULL);
        mem_list.push_back(p);
        //cout << " p = " << p << " size = " << mem_list.size() << " check = " << boolalpha << check << "\n";
    } while (check);

    for(int* p: mem_list) {
        free(p);
    }

    int size = mem_list.size() - 1;
    return size;
}

void printHelloMsg() {
  HAL_Delay(1000);
  HAL_GPIO_TogglePin(LD4_GPIO_Port, LD4_Pin); // User LED
  if(CDC_IsComportOpen()) {
    printf("p: com port is open\r\n");
    HAL_Delay(1000);
    uprintf("up: com port is open\r\n");
    HAL_Delay(1000);
    cout << "c: com port is open\r\n"; HAL_Delay(1000);
  } else {
    printf("p: com port state unknown\r\n");
    HAL_Delay(1000);
    uprintf("up: com port state unknown\r\n");
    HAL_Delay(1000);
    cout << "c: com port state unknown\r\n";
    HAL_Delay(1000);
  }
  HAL_GPIO_TogglePin(LD4_GPIO_Port, LD4_Pin); // User LED

}

// msg support
const unsigned int maxQueueSize = 10000;
//fixedQueue<tickMsg, maxQueueSize> tickMsgBuffer;
std::queue<tickMsg> tickMsgBuffer;

uint32_t time_last = 0;
void addTickMsg(uint32_t spiFlags, HAL_StatusTypeDef status, uint32_t tx, uint32_t rx) {
  uint32_t time_a = __HAL_TIM_GET_COUNTER(&htim2);
  uint32_t time_diff = time_a - time_last;
  tickMsg p{time_a, time_diff, spiFlags, status, tx, rx};
  tickMsgBuffer.push(p);
  time_last = time_a;
}

void dumpTickMsg(int n) {

  int size = tickMsgBuffer.size();
  if (size == 0) {
      return;
  }

  // if n is less than zero, dump all messages in fixed size queue
  //if ((n < 0) || (n > size)) {
  //    n = size;
  //}
  n = size;

  tickMsg p;
  for (int i=0; i < n; ++i) {
    p = tickMsgBuffer.front();
    uprintf("%8d us (delta=%8d us): (tx=%02x rx=%02x) ",
            p.time_a, p.time_diff, p.tx, p.rx);

    bool is_busy = p.spiFlags & SPI_FLAG_BSY;
    bool is_overrun = p.spiFlags & SPI_FLAG_OVR;
    bool is_rx_buffer_empty = ! (p.spiFlags & SPI_FLAG_RXNE);
    bool is_tx_buffer_empty = p.spiFlags & SPI_FLAG_TXE;

    if (is_busy) 
      uprintf("%s BUSY %s", colorMap["red"], colorMap["reset"]); 

    uprintf("\r\n");
    tickMsgBuffer.pop();
  }

}

