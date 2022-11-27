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
        let add_height = |n: &u32| n + (height * sub_perm.len()) as u32;

        forwards.extend(sub_perm.iter().map(add_height));

        backwards.extend(sub_perm.iter().rev().map(add_height))
    }

    (forwards, backwards)
}

fn get_permutation_set(dims: &[usize]) -> Vec<ffi::perm> {
    let primary_dim = match dims.last() {
        Some(&dim) => dim,
        // If there are no dimensions, then there is a single
        // vertex, which can only be "exchanged" with itself.
        None => {
            return vec![ffi::perm { value: vec![0] }];
        }
    };

    let sub_perms = get_permutation_set(&dims[0..dims.len() - 1]);

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

    let mut result = Vec::with_capacity(n_perms);

    for sub_perm in sub_perms.iter() {
        // Create two reference permutations, one where the primary dimension is
        // preserved, and one where it is flipped.
        let (forwards, backwards) = base_perms(n_vertices, primary_dim, &sub_perm.value);
    }

    result
}

#[cxx::bridge(namespace = hrp)]
mod ffi {
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
}
