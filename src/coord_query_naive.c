#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include "record.h"
#include "coord_query.h"

struct naive_data {
    struct record *rs;
    int n;
};

struct naive_data* mk_naive(struct record* rs, int n) {
  struct naive_data *data = malloc(sizeof(struct naive_data));

  data->rs = rs;
  data->n = n;

  return data;
}

void free_naive(struct naive_data* data) {
    free(data);
}

const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {
  if (data->n == 0) {
    return NULL;
  }

  const struct record* closest = &data->rs[0];

  double dx = closest-> lon - lon;
  double dy = closest-> lat - lat;
  double best_distance = sqrt(dx * dx + dy * dy);

  for (int i = 1; i < data->n; i++) {
    dx = data->rs[i].lon - lon;
    dy = data->rs[i].lat - lat;

    double distance = sqrt(dx * dx + dy * dy);

    if (distance < best_distance) {
      best_distance = distance;
      closest = &data->rs[i];
    }
  }
  return closest;
}



int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
