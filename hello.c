#include <hb.h>

int main() {
  hb_buffer_t * buf = hb_buffer_create();
  hb_buffer_add_utf8(buf, "Hello World!", -1, 0, -1);
}
