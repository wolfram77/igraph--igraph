/* -*- mode: C -*-  */
/* vim:set ts=4 sw=4 sts=4 et: */
/*
   IGraph library.
   Copyright (C) 2007-2020  The igraph development team <igraph@igraph.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <igraph.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


int main(int argc, char **argv) {
    char *filename = argv[1];
    FILE *file = fopen(filename, "r");

    igraph_t graph;
    igraph_vector_t membership;
    igraph_real_t modularity;
    struct timeval start, stop;

    printf("Reading graph from %s ...\n", filename);
    igraph_read_graph_edgelist(&graph, file, 0, 0);  /* Read graph from file. */
    printf("Read graph with %zu vertices and %zu edges.\n", (size_t) igraph_vcount(&graph), (size_t) igraph_ecount(&graph));

    /* Label propagation is a stochastic method; the result will depend on the random seed. */
    igraph_rng_seed(igraph_rng_default(), 123);

    /* All igraph functions that returns their result in an igraph_vector_t must be given
       an already initialized vector. */
    igraph_vector_init(&membership, 0);
    gettimeofday(&start, NULL);
    igraph_community_label_propagation(
                &graph, &membership,
                /* weights= */ NULL, /* initial= */ NULL, /* fixed= */ NULL,
                &modularity, IGRAPH_LPA_FAST);
    gettimeofday(&stop, NULL);
    float duration = (stop.tv_sec - start.tv_sec) * 1000.0f + (stop.tv_usec - start.tv_usec) / 1000.0f;

    printf("%ld communities found; modularity score is %g.\n",
           (long int) (igraph_vector_max(&membership) + 1),
           modularity);

    printf("Duration: %f ms\n", duration);

    /* Save the community memberships to a file */
    printf("Saving community memberships to %s ...\n", argv[2]);
    FILE *output = fopen(argv[2], "w");
    for (long int i = 0; i < igraph_vector_size(&membership); i++) {
        fprintf(output, "%ld %ld\n", (long int) i, (long int) VECTOR(membership)[i]);
    }
    fclose(output);

    /* Destroy data structures at the end. */
    igraph_vector_destroy(&membership);
    igraph_destroy(&graph);

    /* Close the file */
    fclose(file);
    return 0;
}
