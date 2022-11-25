fn get_permutation_set(dims: &[usize]) -> Vec<u32> {
    Vec::new()
}

#[cxx::bridge(namespace = hrp)]
mod ffi {
    extern "Rust" {
        fn get_permutation_set(dims: &[usize]) -> Vec<u32>;
    }
}
