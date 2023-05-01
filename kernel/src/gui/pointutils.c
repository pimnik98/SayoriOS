#include "gui/pointutils.h"

/**
 * @param px - Point X
 * @param py - Point Y
 * @param rx - Rect X
 * @param ry - Rect Y
 * @param rw - Rect W
 * @param rh - Rect H
*/
bool point_in_rect(ssize_t px, ssize_t py, ssize_t rx, ssize_t ry, ssize_t rw, ssize_t rh) {
  return (px >= rx &&
      px <= rx + rw &&
      py >= ry &&
      py <= ry + rh);
}