/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#include "external_spinner.h"

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

namespace
{

void write_pid(int fd, pid_t pid)
{
    auto const pid_str = std::to_string(pid);
    size_t nwritten = 0;

    while (nwritten < pid_str.size())
    {
        auto const n = write(fd, pid_str.data() + nwritten, pid_str.size() - nwritten);
        if (n < 0)
        {
            if (errno != EINTR)
                return;
        }
        else if (n == 0)
        {
            return;
        }
        else
        {
            nwritten += n;
        }
    }
}

pid_t read_pid(int fd)
{
    std::string pid_str;
    bool more_data_to_read = true;

    while (more_data_to_read)
    {
        size_t const bufsize = 256;
        char data[bufsize];
        auto const nread = read(fd, data, bufsize);
        if (nread < 0)
        {
            if (errno != EINTR)
                return 0;
        }
        else if (nread == 0)
        {
            more_data_to_read = false;
        }
        else
        {
            pid_str.append(data, nread);
        }
    }

    return std::atoi(pid_str.c_str());
}

}

usc::ExternalSpinner::ExternalSpinner(
    std::string const& executable,
    std::string const& mir_socket)
    : executable{executable},
      mir_socket{mir_socket},
      spinner_pid{0}
{
}

usc::ExternalSpinner::~ExternalSpinner()
{
    kill();
}

void usc::ExternalSpinner::ensure_running()
{
    std::lock_guard<std::mutex> lock{mutex};

    if (executable.empty() || spinner_pid)
        return;

    /* Use a double fork to avoid zombie processes */
    int pipefd[2] {0,0};
    if (pipe(pipefd)) {}

    auto const pid = fork();
    if (!pid)
    {
        close(pipefd[0]);

        auto const spid = fork();
        if (!spid)
        {
            close(pipefd[1]);
            setenv("MIR_SOCKET", mir_socket.c_str(), 1);
            execlp(executable.c_str(), executable.c_str(), nullptr);
        }
        else
        {
            /* Send the spinner pid to grandparent (USC) */
            write_pid(pipefd[1], spid);
            close(pipefd[1]);
            exit(0);
        }
    }
    else
    {
        close(pipefd[1]);
        waitpid(pid, nullptr, 0);
        /* Get spinner pid from grandchild */
        spinner_pid = read_pid(pipefd[0]);
    }

    close(pipefd[0]);
}

void usc::ExternalSpinner::kill()
{
    std::lock_guard<std::mutex> lock{mutex};

    if (spinner_pid)
    {
        ::kill(spinner_pid, SIGTERM);
        spinner_pid = 0;
    }
}

pid_t usc::ExternalSpinner::pid()
{
    std::lock_guard<std::mutex> lock{mutex};

    return spinner_pid;
}
