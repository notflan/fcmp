use super::*;

/// Represents an open and memory mapped file
#[derive(Debug)]
pub struct MemMap
{
    map: memmap::Mmap,
    file: fs::File,
}

impl MemMap
{
    /// Get the memory mapped portion as a slice
    #[inline] pub fn as_slice(&self) -> &[u8]
    {
	&self.map[..]
    }
}

/// Attempt to map this file
pub fn map(file: impl AsRef<Path>) -> io::Result<MemMap>
{
    let file = OpenOptions::new()
        .read(true)
        .open(file)?;
    Ok(MemMap {
	map: unsafe { memmap::Mmap::map(&file)? },
	file,
    })
}
