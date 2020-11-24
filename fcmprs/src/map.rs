use super::*;

use once_cell::sync::OnceCell;

pub trait MappedFile
{
    #[inline] fn as_slice(&self) -> &[u8]
    {
	&self.as_map()[..]
    }
    fn as_map(&self) -> &memmap::Mmap;
    fn as_file(&self) -> &fs::File;
}

pub trait MappedFileNew: MappedFile + Sized
{
    fn try_map(file: fs::File) -> io::Result<Self>;
    #[inline] fn map(file: fs::File) -> Self
    {
	Self::try_map(file).unwrap()
    }
}

/// Represents an open and memory mapped file
#[derive(Debug)]
pub struct MemMap
{
    map: memmap::Mmap,
    file: fs::File,
}

impl MappedFile for MemMap
{
    /// Get the memory mapped portion as a slice
    fn as_slice(&self) -> &[u8] {
	&self.map[..]
    }
    fn as_map(&self) -> &memmap::Mmap {
	&self.map
    }
    #[inline] fn as_file(&self) -> &fs::File {
	&self.file
    }
}
impl MappedFileNew for MemMap
{
    #[inline] fn try_map(file: fs::File) -> io::Result<Self>
    {
	Ok(MemMap {
	    map: unsafe { memmap::Mmap::map(&file)? },
	    file,
	})
    }
}

/// Attempt to map this file
pub fn map_with<M: MappedFileNew>(file: &Path) -> io::Result<M>
{
    let file = OpenOptions::new()
        .read(true)
        .open(file)?;

    M::try_map(file)
}

/// Type container for memory map
pub type DefaultMapType = LazyMap;

/// Attempt to map this file to the `DefaultMapType`
pub fn map(file: impl AsRef<Path>) -> io::Result<DefaultMapType>
{
    map_with(file.as_ref())
}

/// An open and maybe mapped file
#[derive(Debug)]
pub struct LazyMap
{
    map: OnceCell<memmap::Mmap>,
    file: fs::File,
}

impl LazyMap
{
    #[inline(always)] fn get_map(&self) -> &memmap::Mmap
    {
	self.map.get_or_init(|| unsafe {memmap::Mmap::map(&self.file).expect("Lazy map failed")})
    }
    
    #[inline(always)] fn try_get_map(&self) -> io::Result<&memmap::Mmap>
    {
	self.map.get_or_try_init(|| unsafe {memmap::Mmap::map(&self.file)})
    }
    
    /// Is the memory mapped already?
    #[inline] pub fn is_mapped(&self) -> bool
    {
	self.map.get().is_some()
    }

    /// Get the mapped portion if it is mapped, attempting a map if not
    #[inline] pub fn try_as_slice(&self) -> io::Result<&[u8]>
    {
	Ok(&self.try_get_map()?[..])
    }
}

impl MappedFile for LazyMap
{
    /// Get the memory mapped portion as a slice
    ///
    /// Returns blank slice if mapping fails
    #[inline] fn as_slice(&self) -> &[u8]
    {
	self.try_get_map()
	    .map(|x| &x[..])
	    .unwrap_or(&[])
    }
    fn as_map(&self) -> &memmap::Mmap {
	self.map.get().unwrap()
    }
    #[inline] fn as_file(&self) -> &fs::File {
	&self.file
    }
}
impl MappedFileNew for LazyMap
{
    #[inline] fn try_map(file: fs::File) -> io::Result<Self>
    {
	Ok(LazyMap {
	    map: OnceCell::new(),
	    file,
	})
    }
}
