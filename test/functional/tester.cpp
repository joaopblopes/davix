#include <stdio.h>
#include <iostream>
#include <vector>
#include <boost/program_options.hpp>

#include <davix.hpp>
#include "davix_test_lib.h"

namespace po = boost::program_options;
using namespace Davix;

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()

#define DECLARE_TEST() std::cout << "Performing test: " << __FUNCTION__ << " on " << uri << std::endl

const std::string testString("This is a file generated by davix tests. It is safe to delete.");
const std::string testfile("davix-testfile-");

#define ASSERT(assertion, msg) \
    if((assertion) == false) throw std::runtime_error( SSTR(__FILE__ << ":" << __LINE__ << " (" << __func__ << "): Assertion " << #assertion << " failed.\n" << msg))


void initialization() {
    davix_set_log_level(DAVIX_LOG_ALL);
}

std::vector<std::string> split(const std::string str, const std::string delim) {
    size_t prev = 0, cur;
    std::vector<std::string> results;
    while((cur = str.find(delim, prev)) != std::string::npos) {
        results.push_back(str.substr(prev, cur-prev));
        prev = cur + delim.size();
    }
    std::string last = str.substr(prev, str.size()-prev);
    if(last.size() != 0)
        results.push_back(last);

    return results;
}

namespace Auth {
enum Type {AWS, PROXY, NONE};
Type fromString(const std::string &str) {
    if(str == "aws")
        return Auth::AWS;
    if(str == "proxy")
        return Auth::PROXY;
    if(str == "none")
        return Auth::NONE;

    throw std::invalid_argument(SSTR(str << " not a valid authentication method"));
};
};

po::variables_map parse_args(int argc, char** argv) {
    po::options_description desc("davix functional tests runner");

    desc.add_options()
        ("help", "produce help message")
        ("auth", po::value<std::string>()->default_value("none"), "authentication method to use (proxy, aws, none)")
        ("s3accesskey", po::value<std::string>(), "s3 access key")
        ("s3secretkey", po::value<std::string>(), "s3 secret key")
        ("s3region", po::value<std::string>(), "s3 region")
        ("s3alternate", "s3 alternate")
        ("cert", po::value<std::string>(), "path to the proxy certificate to use")
        ("uri", po::value<std::string>(), "uri to test against")
        ("command", po::value<std::vector<std::string> >()->multitoken(), "test to run")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        exit(1);
    }
    return vm;
}

std::string opt(const po::variables_map &vm, const std::string key) {
    return vm[key].as<std::string>();
}

void authentication(const po::variables_map &vm, const Auth::Type &auth, RequestParams &params) {
    if(auth == Auth::AWS) {
        params.setProtocol(RequestProtocol::AwsS3);

        if(vm.count("s3accesskey") == 0) throw std::invalid_argument("--s3accesskey not specified");
        if(vm.count("s3secretkey") == 0) throw std::invalid_argument("--s3secretkey not specified");

        params.setAwsAuthorizationKeys(opt(vm, "s3secretkey"), opt(vm, "s3accesskey"));
        if(vm.count("s3region") != 0) params.setAwsRegion(opt(vm, "s3region"));
        if(vm.count("s3alternate") != 0) params.setAwsAlternate(true);
    }
    else if(auth == Auth::PROXY) {
        configure_grid_env("proxy", params);
    }
    else {
        throw std::invalid_argument("unknown authentication method");
    }
}

void makeCollection(const RequestParams &params, Uri uri) {
    DECLARE_TEST();

    Context context;
    DavFile file(context, params, uri);
    file.makeCollection(&params);
    std::cout << "Done!" << std::endl;

    // make sure it is empty
    DavFile::Iterator it = file.listCollection(&params);
    ASSERT(it.name() == "" && !it.next(), "Newly created directory not empty!");
}

void populate(const RequestParams &params, const Uri uri, const int nfiles) {
    DECLARE_TEST();

    for(int i = 1; i <= nfiles; i++) {
        Context context;
        Uri u(uri);
        u.addPathSegment(SSTR(testfile << i));
        DavFile file(context, params, u);
        file.put(NULL, testString.c_str(), testString.size());
        std::cout << "File " << i << " uploaded successfully." << std::endl;
        std::cout << u << std::endl;
    }
}

void remove(const RequestParams &params, const Uri uri) {
    DECLARE_TEST();

    // a very dangerous test.. Make sure that uri at least
    // contains "davix-test" in its path.
    bool safePath = uri.getPath().find("davix-test") != std::string::npos;
    ASSERT(safePath, "Uri given does not contain the string 'davix-test'. Refusing to perform delete operation for safety.");

    Context context;
    DavFile file(context, params, uri);
    file.deletion(&params);
}

int run(int argc, char** argv) {
    RequestParams params;
    params.setOperationRetry(3);

    po::variables_map vm = parse_args(argc, argv);
    Auth::Type auth = Auth::fromString(opt(vm, "auth"));

    ASSERT(vm.count("command") != 0, "--command is necessary");
    ASSERT(vm.count("uri") != 0, "--uri is necessary");

    std::vector<std::string> cmd = vm["command"].as<std::vector<std::string> >();
    Uri uri = Uri(opt(vm, "uri"));

    authentication(vm, auth, params);

    if(cmd[0] == "makeCollection") {
        ASSERT(cmd.size() == 1, "Wrong number of arguments to makeCollection");
        makeCollection(params, uri);
    }
    else if(cmd[0] == "populate") {
        ASSERT(cmd.size() == 2, "Wrong number of arguments to populate");
        populate(params, uri, atoi(cmd[1].c_str()));
    }
    else if(cmd[0] == "remove") {
        ASSERT(cmd.size() == 1, "Wrong number of arguments to remove");
        remove(params, uri);
    }
    else {
        ASSERT(false, "Unknown command: " << cmd[0]);
    }
}

int main(int argc, char** argv) {
    try {
        run(argc, argv);
    }
    catch(std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}