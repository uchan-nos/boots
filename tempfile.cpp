#include "tempfile.hpp"

using namespace boost;

TemporaryFile::TemporaryFile()
    : path_(filesystem::unique_path())
{}

TemporaryFile::~TemporaryFile()
{
    filesystem::remove(path_);
}

const boost::filesystem::path& TemporaryFile::path() const
{
    return path_;
}
