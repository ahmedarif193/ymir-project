#ifndef LINUX_H
#define LINUX_H

#include "string.h"
#include "vector.h"
#include "map.h"

#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
namespace lxcd {

#if __WORDSIZE == 64
typedef unsigned long long time_t;
#else
typedef unsigned long time_t;
#endif
inline double getFileSize(const char *filename) {
	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
		perror("Cannot open file");
		return -1;
	}

	struct stat st;
	if (fstat(fd, &st) == 0) {
		// Convert size in bytes to size in megabytes
		close(fd);
		return (double)st.st_size / (1024 * 1024);
	} else {
		// Error handling
		perror("Cannot determine size of file");
		close(fd);
		return -1;
	}
}
/**
 * @brief Executes a command in the system's command-line interface and captures its output.
 *
 * This function executes a given command using popen and captures the output. It returns
 * the output as a string and also provides the return code of the executed command.
 *
 * @param cmd The command to be executed.
 * @param retcode Reference to an integer where the return code of the command will be stored.
 * @return lxcd::string The output of the executed command.
 * @throws RuntimeError if popen() fails to execute the command.
 *
 */
inline lxcd::string exec(lxcd::string cmd, int &retcode) {
	printf("execute the cmd %s", cmd.c_str());
	lxcd::string result = "";
	char buffer[128];
	FILE* pipe = popen(cmd.c_str(), "r");

	if (!pipe) {
		retcode = -1;
		return "";
	}

	while (fgets(buffer, sizeof buffer, pipe) != NULL) {
		result += buffer;
	}

	retcode = WEXITSTATUS(pclose(pipe));
	return result;
}


struct mount {
	string device;
	string mountPoint;
	string fsType;
	string options;

	mount() = default;

	mount(const string& device, const string& mountPoint,
	      const string& fsType, const string& options)
		: device(device), mountPoint(mountPoint), fsType(fsType), options(options) {
	}
};

inline vector<mount> parseProcMounts() {
	FILE* fp = fopen("/proc/mounts", "r");
	if(!fp) {
		//throw RuntimeError("failed to open /proc/mounts");
	}

	vector<mount> entries;

	while(true) {
		// Read the next line from the file
		char buf[1024];
		if(!fgets(buf, sizeof(buf), fp)) {
			break;
		}

		// Split the line into its components
		vector<string> tokens;
		string line(buf);
		size_t pos = 0;
		while(true) {
			size_t nextPos = line.find(" ", pos);
			if(nextPos == string::npos) {
				tokens.push_back(line.substr(pos));
				break;
			} else {
				tokens.push_back(line.substr(pos, nextPos - pos));
				pos = nextPos + 1;
			}
		}

		// Create a mount object from the tokens
		if(tokens.size() >= 4) {
			mount entry(tokens[0], tokens[1], tokens[2], tokens[3]);
			entries.push_back(entry);
		}
	}

	fclose(fp);

	return entries;
}
/**
 * @brief Retrieves a list of mount points from the system's /proc/mounts.
 *
 * Calls another function parseProcMounts to get the mount information,
 * and then extracts the mount points into a vector<string>.
 *
 * @return vector<string> A list of mount point paths as strings.
 */
inline vector<string> getProcMountPoints() {
	auto mounts = parseProcMounts();
	vector<string> mountPoints;
	for(auto it = mounts.begin(); it != mounts.end(); ++it) {
		mountPoints.push_back((*it).mountPoint);
	}
	return mountPoints;
}
/**
 * @brief Checks if a given mount point currently exists in the system's mount points.
 *
 * Retrieves the current mount points using getProcMountPoints and compares
 * each mount point with the provided mountPoint argument. If a match is found,
 * it prints a message to stderr and returns true.
 *
 * @param mountPoint The mount point to check for existence.
 * @return bool True if the mount point exists, otherwise false.
 */
inline bool isMountExist(const string& mountPoint) {
	auto mounts = getProcMountPoints();
	for(auto it = mounts.begin(); it != mounts.end(); ++it) {
		if(it->contains(mountPoint)) {
			printf("Mount point found : %s\n", (*it).c_str());
			return true;
		}
	}
	return false;
}
/**
 * @brief Mounts a squashfs filesystem at a specified mount point.
 *
 * First checks if the mount point already exists using isMountExist.
 * If the mount point does not exist, it executes the mount command
 * using the system function.
 *
 * @param path The path to the squashfs filesystem to mount.
 * @param mountPoint The mount point where the filesystem will be mounted.
 */
inline void mountSquashfs(const string& path, const string& mountPoint) {
	printf("trying to mount : %s\n", path.c_str());
	if(!isMountExist(mountPoint)) {
		// Not mounted, mount it
		string command = "mount -t squashfs " + path + " " + mountPoint;
		int ret = system(command.c_str());
		printf("mounting finished with ret : %d\n", ret);
	}
}
/**
 * @brief Unmounts a squashfs filesystem from a specified mount point.
 *
 * Checks if the mount point exists using isMountExist. If it exists,
 * it executes the umount command using the system function.
 *
 * @param mountPoint The mount point where the filesystem will be unmounted.
 */
inline void umountSquashfs(const string& mountPoint) {
	printf("trying to umount : %s\n", mountPoint.c_str());
	string command = "umount -f " + mountPoint + " 2> /dev/null";
	int ret = system(command.c_str());
	printf("umounting finished with ret : %d\n", ret );
}
class Process {
public:
Process(const string& cmd) : cmd_(cmd) {
}

int run() {
	return system(cmd_.c_str());
}

private:
string cmd_;
};

inline int create_directories(const lxcd::string& path) {
	if (path.empty()) {
		return 0;
	}

	// Assuming split returns a vector of lxcd::string
	lxcd::vector<lxcd::string> parts = path.split('/');

	lxcd::string currentPath;
	if (path[0] == '/') {
		currentPath = "/"; // Handle absolute paths
	}

	for (const auto& part : parts) {
		if (!part.empty()) {
			currentPath += part + "/";
			//printf("LOG mountSquashfs %s\n", currentPath.c_str());
			mkdir(currentPath.c_str(), 0777);
		}
	}
	return 0;
}

class CProcess {
public:
// Constructors
CProcess() : processRunning(new bool(false)), pid(new pid_t(0)), exitCode(new int(0)), output(new lxcd::string()) {
}
CProcess(const lxcd::string &containerName) : containerName(containerName), processRunning(new bool(false)), pid(new pid_t(0)), exitCode(new int(0)), output(new lxcd::string()) {
}
CProcess(const CProcess& other) : containerName(other.containerName), processRunning(new bool(*(other.processRunning))), pid(new pid_t(*(other.pid))), exitCode(new int(*(other.exitCode))), output(new lxcd::string(*(other.output))) {
}

~CProcess() {
	printf("[CProcess] Destructor called. Stopping process...\n");
	stop();
}

// Public Methods
void setContainer(const lxcd::string &containerName){
	this->containerName = containerName;
}

void runAsync(const lxcd::string& command, void (*callback)(int, const lxcd::string&, void*), void* priv) {
	printf("[Info] entering runAsync with pthread.\n");
	*processRunning = true;

	pthread_t thread_id;
	ThreadData* data = new ThreadData{this, command, callback, priv};

	if (pthread_create(&thread_id, NULL, &CProcess::runCommandHelper, data) != 0) {
		printf("[Error] Failed to create thread in runAsync.\n");
		*processRunning = false;
		delete data;
		return;
	}

	printf("[Info] Child thread started.\n");
}

void run(const lxcd::string& command) {
	*processRunning = true;
	runCommand(command);
	*processRunning = false;
}

bool isRunning() const {
	return *processRunning;
}

pid_t getPid() const {
	return *pid;
}

const lxcd::string& getOutput() const {
	return *output;
}

int getExitCode() const {
	return *exitCode;
}

void stop() {
	if (pid && *pid > 0) {
		printf("[Info] Stopping process with PID: %d\n", *pid);
		kill(*pid, SIGTERM);
		waitpid(*pid, nullptr, 0);
	}
	*processRunning = false;
}

private:
struct ThreadData {
	CProcess* process;
	lxcd::string command;
	void (*callback)(int, const lxcd::string&, void*);
	void* priv;
};
static void* runCommandHelper(void* arg) {
	ThreadData* data = static_cast<ThreadData*>(arg);

	data->process->runCommand(data->command);
	data->callback(*(data->process->exitCode), *(data->process->output), data->priv);

	delete data;
	pthread_exit(NULL);
}

void runCommand(const lxcd::string& commandVec) {
	printf("[Info] Entering runCommand.\n");
	struct lxc_container *containerObj = lxc_container_new(containerName.c_str(), NULL);
	if (!containerObj) {
		printf("[Error] Failed to allocate LXC container object.\n");
		*processRunning = false;
		return;
	}
	if (containerObj->is_running(containerObj)) {
		executeCommandInContainer(containerObj, commandVec);
	} else {
		printf("[Warning] Container '%s' is not running.\n", containerName.c_str());
		*processRunning = false;
	}
	printf("[Info] Exiting runCommand.\n");
}

void executeCommandInContainer(struct lxc_container *containerObj, const lxcd::string& commandVec) {
	lxcd::string cmd;
	lxcd::vector<lxcd::string> parts = commandVec.split(' ');

	if (parts.size() > 1) {
		cmd = parts[0];
		// Convert parts to char* array (argv)
		char** argv = new char*[parts.size() + 1]; // +1 for NULL terminator

		// First element is the command
		argv[0] = const_cast<char*>(cmd.c_str());

		// Copy arguments
		for (size_t i = 1; i < parts.size(); ++i) {
			argv[i] = const_cast<char*>(parts[i].c_str());
		}

		// NULL terminator
		argv[parts.size()] = nullptr;

		// Now, you can use cmd and argv as needed
		lxc_attach_command_t command = {argv[0], argv};

		lxc_attach_options_t attach_options = LXC_ATTACH_OPTIONS_DEFAULT;
		if (containerObj->attach(containerObj, lxc_attach_run_command, &command, &attach_options, pid.get()) < 0) {
			printf("Failed to attach and run the command");
		}
		delete[] argv;
	} else {
		// Handle the case with only the command
		char* argv[] = {const_cast<char*>(commandVec.c_str()), nullptr};
		lxc_attach_command_t command = {argv[0], argv};

		lxc_attach_options_t attach_options = LXC_ATTACH_OPTIONS_DEFAULT;
		if (containerObj->attach(containerObj, lxc_attach_run_command, &command, &attach_options, pid.get()) < 0) {
			printf("Failed to attach and run the command");
		}
	}

	int status;
	waitpid(*pid, &status, 0);
	if (WIFEXITED(status)) {
		*exitCode = WEXITSTATUS(status);
	}
}

lxcd::SharedPtr<lxcd::string> output;
lxcd::SharedPtr<int> exitCode;
lxcd::SharedPtr<bool> processRunning;
lxcd::string containerName;
lxcd::SharedPtr<pid_t> pid;
};


} // namespace lxcd

#endif
