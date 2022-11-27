use itertools::Itertools;

fn factorial(n: usize) -> usize {
    if n <= 1 {
        1
    } else {
        n * factorial(n - 1)
    }
}

fn n_permutations(dims: &[usize]) -> usize {
    // Get the frequencies of each dimension
    let mut dim_counts = dims.iter().counts();

    // If there are any ones, remove them. Store the frequency (0 if not present).
    let n_ones = dim_counts.remove(&1).unwrap_or(0);

    // Once ones are excluded, the formula is 2 ^ n * factorial(k1) * factorial(k2) * ...
    // (where k1, k2, etc are the frequences)
    dim_counts
        .values()
        .fold(1 << (dims.len() - n_ones), |init, count| {
            init * factorial(*count)
        })
}

// Computes the partial products such that partial_products(src) =
// (1, src[0], (src[0] * src[1]), ... , (product(src)))
fn partial_products(src: &[usize]) -> Vec<usize> {
    let mut running_product = 1;
    let mut result: Vec<usize> = src
        .iter()
        .map(|n| {
            let result = running_product;
            running_product *= n;
            result
        })
        .collect();
    result.push(running_product);

    result
}

// Constructs a pair of permutations (forwards, backwards) by extending `sub_perm`
// upwards `primary_dims` times, adjusting indices per extension accordingly.
// `n_vertices` is used for the capacity of the results.
fn base_perms(n_vertices: usize, primary_dim: usize, sub_perm: &[u32]) -> (Vec<u32>, Vec<u32>) {
    let mut forwards = Vec::with_capacity(n_vertices);
    let mut backwards = Vec::with_capacity(n_vertices);

    for height in 0..primary_dim {
        forwards.extend(
            sub_perm
                .iter()
                .map(|n: &u32| n + (height * sub_perm.len()) as u32),
        );

        backwards.extend(
            sub_perm
                .iter()
                .map(|n: &u32| n + ((primary_dim - height - 1) * sub_perm.len()) as u32),
        )
    }

    (forwards, backwards)
}

// Returns the coordinate of a dimension in `value` with given height and size.
// (Height is how tall the dimension itself is, the size is the number of vertices
//  that would present if all higher dimensions were omitted. For example, if the
//  dimensions are [2,2,2], all heights are 2 and the sizes are 1, 2, and 4.)
fn extract_dimension(value: u32, dim_height: usize, dim_size: u32) -> u32 {
    (value / dim_size) % dim_height as u32
}

fn get_permutation_set_impl(dims: &[usize]) -> Vec<Vec<u32>> {
    let primary_dim = match dims.last() {
        Some(&dim) => dim,
        // If there are no dimensions, then there is a single
        // vertex, which can only be "exchanged" with itself.
        None => {
            return vec![vec![0]];
        }
    };

    let sub_perms = get_permutation_set_impl(&dims[0..dims.len() - 1]);

    // Ones provide no additional vertices, and cause complications in the
    // algorithm. We can just omit them entirely.
    if primary_dim == 1 {
        return sub_perms;
    }

    let n_perms = n_permutations(&dims);

    let dim_partial_products = partial_products(&dims);

    let n_vertices = *dim_partial_products
        .last()
        .expect("partial_products should return a non-empty vector.");

    let sub_n_vertices = *dim_partial_products
        .get(dim_partial_products.len() - 2)
        .expect("partial_products should return a vector of at least size 2.")
        as u32;

    let mut result = Vec::with_capacity(n_perms);

    for sub_perm in sub_perms.iter() {
        // Create two reference permutations, one where the primary dimension is
        // preserved, and one where it is flipped.
        let (forwards, backwards) = base_perms(n_vertices, primary_dim, &sub_perm);

        // Then, for each dimension with the same length as the primary dimension
        // (other than the primary dimension itself), create a new permutation that
        // swaps that dimension with the primary.
        let equal_dims = dims
            .iter()
            .dropping_back(1)
            .zip(dim_partial_products.iter())
            .filter(|(&dim_height, _)| dim_height == primary_dim);

        for (dim_height, dim_size) in equal_dims {
            let dim_size = *dim_size as u32;
            let create_swapped_perm = |base_perm: &[u32]| -> Vec<u32> {
                base_perm
                    .iter()
                    .map(|&value| {
                        let dim_value = extract_dimension(value, *dim_height, dim_size);
                        let primary_dim_value = value / sub_n_vertices as u32;

                        value + (dim_value * sub_n_vertices + primary_dim_value * dim_size)
                            - (primary_dim_value * sub_n_vertices + dim_value * dim_size)
                    })
                    .collect()
            };

            result.push(create_swapped_perm(&forwards));
            result.push(create_swapped_perm(&backwards));
        }

        result.push(forwards);
        result.push(backwards);
    }

    result
}

// Due to limitations with cxx, we cannot return Vec<Vec<u32>> directly.
// To get around this, we wrap the inner Vec in a single-member struct.
fn get_permutation_set(dims: &[usize]) -> Vec<ffi::perm> {
    get_permutation_set_impl(dims)
        .into_iter()
        .map(|value| ffi::perm { value })
        .collect()
}

#[cxx::bridge(namespace = hrp)]
mod ffi {
    #[derive(PartialEq, Eq, Debug)]
    struct perm {
        value: Vec<u32>,
    }
    extern "Rust" {
        fn get_permutation_set(dims: &[usize]) -> Vec<perm>;

        fn n_permutations(dims: &[usize]) -> usize;
    }
}

#[cfg(test)]
mod test {
    use std::collections::HashSet;

    use super::*;

    #[test]
    fn test_factorial() {
        assert_eq!(factorial(0), 1);
        assert_eq!(factorial(1), 1);
        assert_eq!(factorial(2), 2);
        assert_eq!(factorial(3), 6);
        assert_eq!(factorial(4), 24);
        assert_eq!(factorial(5), 120);
        assert_eq!(factorial(6), 720);
    }

    #[test]
    fn test_n_permutations() {
        assert_eq!(n_permutations(&[]), 1);

        assert_eq!(n_permutations(&[1]), 1);
        assert_eq!(n_permutations(&[2]), 2);
        assert_eq!(n_permutations(&[3]), 2);

        assert_eq!(n_permutations(&[1, 1]), 1);
        assert_eq!(n_permutations(&[1, 2]), 2);
        assert_eq!(n_permutations(&[1, 3]), 2);
        assert_eq!(n_permutations(&[2, 1]), 2);
        assert_eq!(n_permutations(&[2, 2]), 8);
        assert_eq!(n_permutations(&[2, 3]), 4);
        assert_eq!(n_permutations(&[3, 1]), 2);
        assert_eq!(n_permutations(&[3, 2]), 4);
        assert_eq!(n_permutations(&[3, 3]), 8);

        assert_eq!(n_permutations(&[1, 2, 1]), 2);
        assert_eq!(n_permutations(&[1, 2, 2]), 8);
        assert_eq!(n_permutations(&[1, 3, 2]), 4);
        assert_eq!(n_permutations(&[2, 1, 2]), 8);
        assert_eq!(n_permutations(&[2, 2, 2]), 48);
        assert_eq!(n_permutations(&[3, 2, 2]), 16);
        assert_eq!(n_permutations(&[2, 3, 2]), 16);
        assert_eq!(n_permutations(&[2, 2, 3]), 16);
        assert_eq!(n_permutations(&[3, 3, 1]), 8);
    }

    #[test]
    fn test_partial_products() {
        assert_eq!(partial_products(&[]), vec![1]);
        assert_eq!(partial_products(&[4]), vec![1, 4]);
        assert_eq!(partial_products(&[5, 3, 2]), vec![1, 5, 15, 30]);
        assert_eq!(partial_products(&[1, 1, 1, 1, 1]), vec![1, 1, 1, 1, 1, 1]);
        assert_eq!(
            partial_products(&[9, 1, 5, 1, 2, 1, 3]),
            vec![1, 9, 9, 45, 45, 90, 90, 270]
        );
    }

    #[test]
    fn test_base_perms() {
        assert_eq!(base_perms(2, 2, &[0]), (vec![0, 1], vec![1, 0]));
        assert_eq!(
            base_perms(4, 2, &[0, 1]),
            (vec![0, 1, 2, 3], vec![2, 3, 0, 1])
        );
        assert_eq!(
            base_perms(6, 3, &[0, 1]),
            (vec![0, 1, 2, 3, 4, 5], vec![4, 5, 2, 3, 0, 1])
        );
        assert_eq!(
            base_perms(8, 2, &[3, 1, 2, 0]),
            (vec![3, 1, 2, 0, 7, 5, 6, 4], vec![7, 5, 6, 4, 3, 1, 2, 0])
        );
        assert_eq!(
            base_perms(12, 3, &[3, 1, 2, 0]),
            (
                vec![3, 1, 2, 0, 7, 5, 6, 4, 11, 9, 10, 8],
                vec![11, 9, 10, 8, 7, 5, 6, 4, 3, 1, 2, 0]
            )
        );
    }

    #[test]
    fn test_extract_dimension() {
        assert_eq!(extract_dimension(0, 2, 1), 0);
        assert_eq!(extract_dimension(1, 2, 1), 1);
        assert_eq!(extract_dimension(2, 2, 1), 0);
        assert_eq!(extract_dimension(3, 2, 1), 1);
        assert_eq!(extract_dimension(4, 2, 1), 0);
        assert_eq!(extract_dimension(5, 2, 1), 1);
        assert_eq!(extract_dimension(6, 2, 1), 0);
        assert_eq!(extract_dimension(7, 2, 1), 1);

        assert_eq!(extract_dimension(0, 2, 2), 0);
        assert_eq!(extract_dimension(1, 2, 2), 0);
        assert_eq!(extract_dimension(2, 2, 2), 1);
        assert_eq!(extract_dimension(3, 2, 2), 1);
        assert_eq!(extract_dimension(4, 2, 2), 0);
        assert_eq!(extract_dimension(5, 2, 2), 0);
        assert_eq!(extract_dimension(6, 2, 2), 1);
        assert_eq!(extract_dimension(7, 2, 2), 1);

        assert_eq!(extract_dimension(0, 2, 4), 0);
        assert_eq!(extract_dimension(1, 2, 4), 0);
        assert_eq!(extract_dimension(2, 2, 4), 0);
        assert_eq!(extract_dimension(3, 2, 4), 0);
        assert_eq!(extract_dimension(4, 2, 4), 1);
        assert_eq!(extract_dimension(5, 2, 4), 1);
        assert_eq!(extract_dimension(6, 2, 4), 1);
        assert_eq!(extract_dimension(7, 2, 4), 1);
    }

    fn as_set(v: Vec<Vec<u32>>) -> HashSet<Vec<u32>> {
        v.into_iter().collect()
    }

    #[test]
    fn test_get_permutation_set() {
        assert_eq!(get_permutation_set_impl(&[]), vec![vec![0]]);
        assert_eq!(get_permutation_set_impl(&[1]), vec![vec![0]]);
        assert_eq!(get_permutation_set_impl(&[1, 1]), vec![vec![0]]);

        assert_eq!(get_permutation_set_impl(&[2]), vec![vec![0, 1], vec![1, 0]]);
        assert_eq!(
            get_permutation_set_impl(&[2, 1]),
            vec![vec![0, 1], vec![1, 0]]
        );
        assert_eq!(
            get_permutation_set_impl(&[1, 2]),
            vec![vec![0, 1], vec![1, 0]]
        );
        assert_eq!(
            get_permutation_set_impl(&[1, 2, 1]),
            vec![vec![0, 1], vec![1, 0]]
        );

        assert_eq!(
            as_set(get_permutation_set_impl(&[2, 2])),
            as_set(vec![
                vec![0, 1, 2, 3],
                vec![0, 2, 1, 3],
                vec![1, 0, 3, 2],
                vec![2, 0, 3, 1],
                vec![1, 3, 0, 2],
                vec![2, 3, 0, 1],
                vec![3, 1, 2, 0],
                vec![3, 2, 1, 0],
            ])
        );

        assert_eq!(
            as_set(get_permutation_set_impl(&[3, 2])),
            as_set(vec![
                vec![0, 1, 2, 3, 4, 5],
                vec![3, 4, 5, 0, 1, 2],
                vec![2, 1, 0, 5, 4, 3],
                vec![5, 4, 3, 2, 1, 0],
            ])
        );

        assert_eq!(
            as_set(get_permutation_set_impl(&[2, 3])),
            as_set(vec![
                vec![0, 1, 2, 3, 4, 5],
                vec![4, 5, 2, 3, 0, 1],
                vec![1, 0, 3, 2, 5, 4],
                vec![5, 4, 3, 2, 1, 0],
            ])
        );

        assert_eq!(
            as_set(get_permutation_set_impl(&[2, 2, 2])),
            as_set(vec![
                vec![0, 1, 2, 3, 4, 5, 6, 7], // Base
                vec![0, 1, 4, 5, 2, 3, 6, 7], //
                vec![0, 2, 1, 3, 4, 6, 5, 7], //
                vec![0, 2, 4, 6, 1, 3, 5, 7], //
                vec![0, 4, 1, 5, 2, 6, 3, 7], //
                vec![0, 4, 2, 6, 1, 5, 3, 7], //
                vec![1, 0, 3, 2, 5, 4, 7, 6], // Reverse dim 0
                vec![1, 0, 5, 4, 3, 2, 7, 6], //
                vec![1, 3, 0, 2, 5, 7, 4, 6], //
                vec![1, 3, 5, 7, 0, 2, 4, 6], //
                vec![1, 5, 0, 4, 3, 7, 2, 6], //
                vec![1, 5, 3, 7, 0, 4, 2, 6], //
                vec![2, 0, 3, 1, 6, 4, 7, 5], //
                vec![2, 0, 6, 4, 3, 1, 7, 5], //
                vec![2, 3, 0, 1, 6, 7, 4, 5], // Reverse dim 1
                vec![2, 3, 6, 7, 0, 1, 4, 5], //
                vec![2, 6, 0, 4, 3, 7, 1, 5], //
                vec![2, 6, 3, 7, 0, 4, 1, 5], //
                vec![3, 1, 2, 0, 7, 5, 6, 4], //
                vec![3, 1, 7, 5, 2, 0, 6, 4], //
                vec![3, 2, 1, 0, 7, 6, 5, 4], // Reverse dim 0, 1
                vec![3, 2, 7, 6, 1, 0, 5, 4], //
                vec![3, 7, 1, 5, 2, 6, 0, 4], //
                vec![3, 7, 2, 6, 1, 5, 0, 4], //
                vec![4, 0, 5, 1, 6, 2, 7, 3], //
                vec![4, 0, 6, 2, 5, 1, 7, 3], //
                vec![4, 5, 0, 1, 6, 7, 2, 3], //
                vec![4, 5, 6, 7, 0, 1, 2, 3], // Reverse dim 2
                vec![4, 6, 0, 2, 5, 7, 1, 3], //
                vec![4, 6, 5, 7, 0, 2, 1, 3], //
                vec![5, 1, 4, 0, 7, 3, 6, 2], //
                vec![5, 1, 7, 3, 4, 0, 6, 2], //
                vec![5, 4, 1, 0, 7, 6, 3, 2], //
                vec![5, 4, 7, 6, 1, 0, 3, 2], // Reverse dim 0, 1
                vec![5, 7, 1, 3, 4, 6, 0, 2], //
                vec![5, 7, 4, 6, 1, 3, 0, 2], //
                vec![6, 2, 4, 0, 7, 3, 5, 1], //
                vec![6, 2, 7, 3, 4, 0, 5, 1], //
                vec![6, 4, 2, 0, 7, 5, 3, 1], //
                vec![6, 4, 7, 5, 2, 0, 3, 1], //
                vec![6, 7, 2, 3, 4, 5, 0, 1], //
                vec![6, 7, 4, 5, 2, 3, 0, 1], // Reverse dim 0, 2
                vec![7, 3, 5, 1, 6, 2, 4, 0], //
                vec![7, 3, 6, 2, 5, 1, 4, 0], //
                vec![7, 5, 3, 1, 6, 4, 2, 0], //
                vec![7, 5, 6, 4, 3, 1, 2, 0], //
                vec![7, 6, 3, 2, 5, 4, 1, 0], //
                vec![7, 6, 5, 4, 3, 2, 1, 0], // Reverse dim 0, 1, 2
            ])
        );
    }
}
