

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
