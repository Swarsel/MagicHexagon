import sys
import subprocess

START_COMMIT = "iAmATeapot/benchmarkCollection"

# Get all git commits of this branch
def getCommits():
    # Get all commits of this branch
    p = subprocess.Popen(['git', 'log', '--pretty=format:"%H"'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    out = out.decode("utf-8")
    # Remove the quotes
    out = out.replace('"', '')
    # Split the commits
    commits = out.split('\n')
    # Remove the last empty commit
    commits.pop()
    return commits

def gitCheckout(commit):
    # Checkout a commit
    p = subprocess.run(['git', 'checkout', commit])
    return

def gitGetMessage():
    # Get the message of a commit
    p = subprocess.Popen(['git', 'log', '-1', '--pretty=%B'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    return out.decode("utf-8").strip()

# Get the runtime of a commit
def getRuntime():
	# Get the runtime of a commit
        p = subprocess.Popen(['make','measure'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = p.communicate()
        err = err.decode("utf-8")
        out = err.split('\n')

        #strip all lines
        out = map(lambda x: x.strip(),out)
        parameters = {}
        #parse runtime
        for line in out:
            line = str(line)
            if "seconds time elapsed" in line:
                line = line.split(' ')
                parameters["runtime"] = line[0]
            elif "instructions:u" in line:
                line = line.split(' ')
                parameters["instructions"] = line[0].replace('\n','').replace('.','')
            elif "cycles:u" in line:
                line = line.split(' ')
                parameters["cycles"] = line[0].replace('\n','').replace(',','')
            elif "branches:u" in line:
                line = line.split(' ')
                parameters["branches"] = line[0].replace('\n','').replace(',','')
            elif "branch-misses:u" in line:
                line = line.split(' ')
                parameters["branch-misses"] = line[0].replace('\n','').replace(',','')
            elif "L1-dcache-load-misses" in line:
                line = line.split(' ')
                parameters["L1-dcache-load-misses"] = line[0].replace('\n','').replace(',','')
        return parameters

commits = ["fc33f7b3bbc42cf7573a1519ec60aa8a6992b744",
"a060402984a4b6d60a2553113c2c4e92587f2495",
"c4d73630a7e1bba0caa83c0bc2a3b33b5d00f962",
"b5ef23cde657bf8c0e16368f900d15516cba5e63",
"17015549c7ab214d5db5dedf5dafd353da9f9313",
"65656bdaf33ab95bed72d4123fb2ee2a8c18a566",
"08444a1ae1de558f07d26697c141e5c04b184cfc",
"795590a57b80940604a4a89d402f5d128ca80275",
"0826f038939ca3bb9c7b9115e7bd24ae91c10072",
"a02a9441e99c5cc0cae77500afd674890c388848",
"dc2ac89d6c3a0bcc33ae6ffc8189031a8c180faa",
"4d734e3aaaa1846c761b051d5f1d7f1b5c6b997b",
"98b7c6272ce46b77eb9569c890199e150668b31c"]

#get files in folder measured runtimes
import os
files = os.listdir("runtimes")
files = list(map(lambda x: x.replace('.csv',''),files))
#filter commits
commits = list(filter(lambda x: x not in files,commits))
print(commits)
for commit in commits:
    gitCheckout(commit)
    print(gitGetMessage())
    subprocess.run(['rm','-r','profdata'])
    subprocess.run(['cp','Makefile','Makefile_old'])
    subprocess.run(['cp','Makefile_new','Makefile'])
    subprocess.run(['make','clean'])
    subprocess.run(['make','measure'], env=dict(os.environ, BUILD="profile"))
    subprocess.run(['make','clean'])
    subprocess.run(['make'], env=dict(os.environ, BUILD="release"))

    for it in range(0,5):
        res = getRuntime()
        if(res == {}):
            print(f"Error on commit {commit}")
            continue
        #write to file
        with open(f"runtimes/{commit}.csv", "a+") as myfile:
            #write header if not there
            if myfile.tell() == 0:
                message = gitGetMessage().replace('\n','')
                myfile.write(f"#{message}\n")
                myfile.write("runtime,instructions,cycles,branches,branch-misses,L1-dcache-load-misses\n")
            #write data
            myfile.write(f"{res['runtime']},{res['instructions']},{res['cycles']},{res['branches']},{res['branch-misses']},{res['L1-dcache-load-misses']}\n")
    subprocess.run(['cp','Makefile_old','Makefile'])

subprocess.Popen(['git', 'checkout', START_COMMIT], stdout=subprocess.PIPE)
