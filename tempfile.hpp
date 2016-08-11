#ifndef TEMPFILE_HPP_
#define TEMPFILE_HPP_

#include <boost/filesystem.hpp>

class TemporaryFile
{
    boost::filesystem::path path_;
public:
    TemporaryFile();
    virtual ~TemporaryFile();
    const boost::filesystem::path& path() const;
};

#endif
