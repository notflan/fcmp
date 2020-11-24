use rayon::prelude::*;
use std::{
    path::Path,
    io, fs::{self, OpenOptions,},
};
use smallvec::SmallVec;

fn usage() -> !
{
    eprintln!("fcmprs: Compare files for identity");
    eprintln!("Usage: {} <files...>", std::env::args().next().unwrap());

    std::process::exit(-1)
}

mod error;
use error::ResultPrintExt as _;

mod map;

fn main() {
    let (map1, rest) = {	
	let mut args = std::env::args().skip(1);
	if let Some(one) = args.next() {
	    (one, args)
	} else {
	    usage();
	}
    };

    std::process::exit(dbg!(if let Some(map1) = map::map(&map1).discard_msg(format!("Failed to map file {}", map1)) {
	let slice = map1.as_slice();
	let mut ok = true;
	let chk: SmallVec<[_; 32]> = rest.filter_map(|filename| {
	    let path = Path::new(&filename);
	    if path.exists() && path.is_file() {
		map::map(path).discard_msg(format!("Failed to map file {}", filename))
		    .or_else(|| { ok = false; None })
	    } else {
		eprintln!("File {} does not exist or is not a normal file", filename);
		ok = false;
		None
	    }
	}).collect();

	if !ok {
	    -1
	} else {
	    match chk.into_par_iter()
		.map(|map| slice == map.as_slice())
		.reduce_with(|x, y| x && y)
	    {
		Some(true) => 0,
		Some(false) => 1,
		None => usage(),
	    }
	}
    } else {
	-1
    }))
}
