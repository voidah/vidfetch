#include <iostream>
#include <string>
#include <quvi/quvi.h>
#include <cstdlib>

struct MediaInfo
{
    bool IsValid;

    std::string Url;
    std::string Title;
    std::string Id;
    int ContentLength;
    std::string ContentType;
    std::string Extension;
    int Duration;

    MediaInfo() : IsValid(false) {}
};


MediaInfo ExtractMediaFromUrl(const std::string& url)
{
    MediaInfo info;

    quvi_t q;
    quvi_init(&q);

    //char *avail_formats;
    //quvi_query_formats(q, (char*)url.c_str(), &avail_formats);
    //std::cout << avail_formats << std::endl;

    // Get the best possible quality
    quvi_setopt(q, QUVIOPT_FORMAT, "best"); // Could be any of the avail_formats list

    quvi_media_t m;
    if(quvi_parse(q, (char*)url.c_str(), &m) != QUVI_OK)
        return info;

    char* prop;
    quvi_getprop(m, QUVIPROP_MEDIAURL, &prop);
    info.Url = (prop ? prop : "");

    quvi_getprop(m, QUVIPROP_PAGETITLE, &prop);
    info.Title = (prop ? prop : "");

    quvi_getprop(m, QUVIPROP_MEDIAID, &prop);
    info.Id = (prop ? prop : "");

    //quvi_getprop(m, QUVIPROP_MEDIACONTENTLENGTH, &prop);
    //info.ContentLength = (prop ? prop : "");

    quvi_getprop(m, QUVIPROP_MEDIACONTENTTYPE, &prop);
    info.ContentType = (prop ? prop : "");

    quvi_getprop(m, QUVIPROP_FILESUFFIX, &prop);
    info.Extension = (prop ? prop : "");

    //quvi_getprop(m, QUVIPROP_MEDIADURATION, &prop);
    //info.Duration = (prop ? prop : "");

    //std::cout << "video url: " << mediaUrl << std::endl;
    quvi_parse_close(&m);
    quvi_close(&q);

    info.IsValid = true;
    return info;
}

void ShowUsage()
{
    std::cout << "Usage: vidfetch [-format] <url>" << std::endl;
    std::cout << "\treplace 'format' by the destination file format (the file extension)" << std::endl;
    std::cout << "\tIf no format is specified, no conversion occurs" << std::endl;
}

int main(int argc, char **argv)
{
    if(argc == 2 && std::string(argv[1]) == "--help")
    {
        ShowUsage();
        return 0;
    }

    if(argc < 2)
    {
        ShowUsage();
        return 1;
    }

    std::string format;
    std::string url;

    for(int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if(arg[0] == '-')
            format = arg.substr(1);
        else
            url = arg;
    }

    MediaInfo info = ExtractMediaFromUrl(url);
    if(info.IsValid)
    {
        std::cout << info.Url << std::endl;
        std::cout << info.Title << std::endl;
        std::cout << info.Extension << std::endl;
    }

    std::string origFilename = info.Title + "." + info.Extension;
    system(("curl -C - \"" + info.Url + "\" -o \"" + origFilename + "\"").c_str());

    // If conversion needed (if a conversion format has been specified):
    if(format != "")
    {
        std::string finalFilename = info.Title + "." + format;
        
        system(("ffmpeg -i \"" + origFilename + "\" -qscale 0 \"" + finalFilename + "\"").c_str());

        // Remove original
        system(("rm \"" + origFilename + "\"").c_str());
    }
}
