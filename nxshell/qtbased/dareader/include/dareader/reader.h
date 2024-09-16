#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef u_int32_t DA_img_type;

  extern const DA_img_type DA_img_type_RGB24;
  extern const DA_img_type DA_img_type_GRAY8;
  extern const DA_img_type DA_img_type_BITMAP;
  extern const DA_img_type DA_img_type_JPEG;

  /*
   * `DA_img`: represent a image.
   *
   * this basically a c-wapper for opencv's IplImage.
   */
  typedef struct DA_img {
      u_int32_t   width, height, depth, channel;
      size_t      size;
      char       *data;
      DA_img_type type;
  } DA_img;

  /*
   * `read_frames_cb`: Function to call when new frame come in.
   *
   * `img` is your new frame.
   *
   * NOTE:
   * **DO NOT** free neither img->data nor img
   */
  typedef void (*DA_read_frames_cb)(
    void * const context, const DA_img * const img);

  /*
   * `read_frames`: Register a `cb` to continuously read data from a `fd`.
   *
   * NOTE:
   * this will start a new thread
   */
  void DA_read_frames(int fd, void * const context, DA_read_frames_cb cb);
#ifdef __cplusplus
}  // end of extern "C"
#endif
