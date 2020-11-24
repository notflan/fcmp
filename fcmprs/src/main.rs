#[cfg(feature="threads")] use rayon::prelude::*;
use std::{
    path::Path,
    io, fs::{self, OpenOptions,},
};
use smallvec::SmallVec;
use cfg_if::cfg_if;

fn usage() -> !
{
    eprintln!("fcmprs: Compare files for identity");
    eprintln!("Usage: {} <files...>", std::env::args().next().unwrap());

    std::process::exit(-1)
}

mod error;
use error::ResultPrintExt as _;

mod map;

struct UnmatchError;

fn main() {
    let (map1, rest) = {	
	let mut args = std::env::args().skip(1);
	if let Some(one) = args.next() {
	    (one, args)
	} else {
	    usage();
	}
    };

    std::process::exit({
	if let Some(map1) = map::map(&map1).discard_msg(format!("Failed to map file {}", map1)) {
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

		cfg_if! {
		    if #[cfg(feature="threads")] {
			match chk.into_par_iter()
			    .map(|map| {
				if slice == map.as_slice() {
				    Ok(())
				}else{
				    Err(UnmatchError)
				}
			    })
			    .try_reduce_with(|_, _| Ok(()))
			{
			    Some(Ok(_)) => 0,
			    Some(Err(_)) => 1,
			    None => usage(),
			}
		    } else {
			match chk.into_iter()
			    .map(|map| {
				slice == map.as_slice()
			    })
			    .try_fold((false, true), |(_, a), b| if a && b {Ok((true, true))} else {Err(UnmatchError)})
			{
			    Ok((true, _)) => 0,
			    Ok((false, _)) => usage(),
			    Err(_) => 1,
			}

		    }
		}
	    }
	} else {
	    -1
	}
    })
}
