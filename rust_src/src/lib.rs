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
