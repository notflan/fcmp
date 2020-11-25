use std::{fmt,error};

#[derive(Debug)]
/// There was a non-matching file
pub enum UnmatchError
{
    Size,
    Data,
    Unknown,
}

impl error::Error for UnmatchError{}
impl fmt::Display for UnmatchError
{
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result
    {
	match self {
	    Self::Size => write!(f, "size differs"),
	    Self::Data => write!(f, "data differs"),
	    _ => write!(f, "unknown error"),
	}
    }
}

pub trait ResultPrintExt<T>
{
    fn discard_msg(self, msg: impl AsRef<str>) -> Option<T>;
}

impl<T, E> ResultPrintExt<T> for Result<T,E>
where E: std::fmt::Display
{
    fn discard_msg(self, msg: impl AsRef<str>) -> Option<T> {
	match self {
	    Ok(v) => Some(v),
	    Err(e) => {
		eprintln!("{}: {}", msg.as_ref(), e);
		None
	    },
	}
    }
}
