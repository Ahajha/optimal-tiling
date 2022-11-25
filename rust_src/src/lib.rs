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

fn get_permutation_set(dims: &[usize]) -> Vec<Vec<u32>> {
    // If all dimensions are 0, then there is a single vertex,
    // which can only be "exchanged" with itself.
    if dims.is_empty() {
        return vec![vec![0]];
    }
    // From here, it can be assumed there is at least one dimension

    let primary_dim = *dims.last().expect("There should be at least one dimension");

    let sub_permutations = get_permutation_set(&dims[0..dims.len() - 1]);

    // Ones provide no additional vertices, and cause complications in the
    // algorithm. We can just omit them entirely.
    if primary_dim == 1 {
        return sub_permutations;
    }

    let n_perms = n_permutations(&dims);

    let dim_partial_products = partial_products(&dims);

    let _n_vertices = *dim_partial_products
        .last()
        .expect("partial_products should return a non-empty vector.");

    Vec::with_capacity(n_perms)
}

#[cxx::bridge(namespace = hrp)]
mod ffi {
    extern "Rust" {
        fn get_permutation_set(dims: &[usize]) -> Vec<u32>;

        fn n_permutations(dims: &[usize]) -> usize;
    }
}
