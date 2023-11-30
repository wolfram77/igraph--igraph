/* -*- mode: C -*-  */
/*
   IGraph library.
   Copyright (C) 2006-2012  Gabor Csardi <csardi.gabor@gmail.com>
   334 Harvard street, Cambridge, MA 02139 USA

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc.,  51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA

*/

#include <igraph.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


int main(int argc, char **argv) {
    char *filename    = argv[1];
    char *outfilename = argv[2];
    FILE *file = fopen(filename, "r");
    FILE *fout = fopen(outfilename, "w");

    igraph_t graph;
    igraph_vector_int_t membership;
    igraph_vector_int_t degree;
    igraph_vector_t weights;
    igraph_integer_t nb_clusters = 0, i = 0;
    igraph_real_t quality = 0, modularity = 0;
    struct timeval start, stop;

    /* Set default seed to get reproducible results */
    igraph_rng_seed(igraph_rng_default(), 0);

    /* Simple unweighted graph */
    printf("Reading graph from %s ...\n", filename);
    igraph_read_graph_edgelist(&graph, file, 0, 0);  /* Read graph from file. */
    printf("Read graph with %zu vertices and %zu edges.\n", (size_t) igraph_vcount(&graph), (size_t) igraph_ecount(&graph));

    /* Initialize membership vector to use for storing communities */
    igraph_vector_int_init(&membership, igraph_vcount(&graph));

    /* Initialize degree vector to use for optimizing modularity */
    igraph_vector_int_init(&degree, igraph_vcount(&graph));
    igraph_degree(&graph, &degree, igraph_vss_all(), IGRAPH_ALL, 1);
    igraph_vector_init(&weights, igraph_vector_int_size(&degree));
    for (i = 0; i < igraph_vector_int_size(&degree); i++) {
        VECTOR(weights)[i] = VECTOR(degree)[i];
    }

    /* Perform Leiden algorithm using modularity until stable iteration */
    gettimeofday(&start, NULL);
    igraph_community_leiden(&graph, NULL, &weights, 1.0 / (2 * igraph_ecount(&graph)), 0.01, 0, -1, &membership, &nb_clusters, &quality);
    // igraph_community_leiden(&graph, NULL, NULL, 0.05, 0.01, 0, 1, &membership, &nb_clusters, &quality);
    // igraph_community_leiden(&graph, NULL, NULL, 0.05, 0.01, 1, 10, &membership, &nb_clusters, &quality);
    gettimeofday(&stop, NULL);
    float duration = (stop.tv_sec - start.tv_sec) * 1000.0f + (stop.tv_usec - start.tv_usec) / 1000.0f;

    printf("Leiden found %" IGRAPH_PRId " clusters using modularity, quality is %.4f.\n", nb_clusters, quality);
    printf("Duration: %f ms\n", duration);

    /* Also calculate the modularity of the partition separately */
    igraph_modularity(
        &graph, &membership, /* weights= */ NULL, /* resolution = */ 1,
        /* directed= */ 0, &modularity);
    printf("Modularity: %f\n", modularity);

    /* Write communities to file */
    for (i = 0; i < igraph_vcount(&graph); ++i) {
        fprintf(fout, "%zu %zu\n", (size_t) i, (size_t) VECTOR(membership)[i]);
    }
    printf("Written communities to %s\n", outfilename);

    igraph_vector_destroy(&weights);
    igraph_vector_int_destroy(&degree);
    igraph_vector_int_destroy(&membership);
    igraph_destroy(&graph);

    /* Close the files */
    fclose(file);
    fclose(fout);
    return 0;
}
