#define BOOST_FILESYSTEM_VERSION 3     // ...
#define BOOST_FILESYSTEM_NO_DEPRECATED // ...
#include <boost/filesystem.hpp>

#include "movie.h"

Movie::Movie(const std::string& dir, const std::string& regExString) :
	dir{new std::string{dir}}, frames{}, bitmapBuffer{}, terminateThread{false}
{
	using namespace boost::filesystem;

	// Append a directory separator character if necessary, so concatenation works as expected.
	if (*--this->dir->end() != path::preferred_separator) *this->dir += path::preferred_separator;

	try
	{
		path path{*this->dir};
		if (exists(path) && is_directory(path))
		{
			boost::regex regEx{regExString, boost::regex::perl};
			boost::cmatch matches;
			for (directory_iterator i{path}; i != directory_iterator{}; ++i)
			{
				if (is_regular_file(i->status()) &&
				    boost::regex_search(i->path().filename().generic_string().c_str(), matches, regEx))
				{
					frames.push_back(std::move(Frame{this->dir, i->path().filename()}));
				}
			}
		}
	}
	catch (const filesystem_error&)
	{
		throw;
	}

	//// <_..._> ////
	///

	try {
		thread = boost::thread{&Movie::populateBuffer, this};
	} catch (...) {
		// ...
	}

	///
	//// </_..._> ////
}

Movie::~Movie()
{
	{
		boost::lock_guard<boost::mutex> lock{terminateFlagAccess};
		terminateThread = true;
	}
	thread.join();
}

Movie& Movie::operator=(Movie&& movee)
{
	dir =          std::move(movee.dir);
	frames =       std::move(movee.frames);
	bitmapBuffer = std::move(movee.bitmapBuffer);

	return *this;
}

void Movie::populateBuffer()
{
	for (std::size_t i = 0; i < size() && bitmapBuffer.size() < 1024; ++i)
	{
		bitmapBuffer.push_front(frames[i].getBitmap());
		frames[i].setBitmap(bitmapBuffer.front());
		{
			boost::lock_guard<boost::mutex> lock{terminateFlagAccess};
			if (terminateThread) break;
		}
	}
}
