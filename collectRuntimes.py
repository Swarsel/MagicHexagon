import sys
import subprocess

START_COMMIT = "main"

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
    p = subprocess.Popen(['git', 'checkout', commit], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    return out

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

commits = getCommits()
#get files in folder measured runtimes
import os
files = os.listdir("measuredRuntimes")
files = list(map(lambda x: x.replace('.csv',''),files))
#filter commits
commits = list(filter(lambda x: x not in files,commits))

for commit in commits[0:1]:
    gitCheckout(commit)
    print(gitGetMessage())
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

subprocess.Popen(['git', 'checkout', START_COMMIT], stdout=subprocess.PIPE)