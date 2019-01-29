<#

.SYNOPSIS
This is a PowerShell script to run all autodiff benchmarking tests.

.DESCRIPTION
This script loops through each of a set of tools (defined using the Tool class) and runs a set of test functions on each of them.

.EXAMPLE
./run-all.ps1 "Release" 10 10 180 600 "C:/path/to/tmp/" $FALSE
This will:
- run only release builds
- aim to run 10 tests of each function, and 10 tests of the derivative of each function
- stop (having completed a whole number of tests) at any point after 180 seconds
- allow each program a maximum of 600 seconds to run all tests
- output results to "C:/path/to/tmp/"
- not repeat any tests for which there already exist a results file

.NOTES
See below for adding new tools or tests.

.LINK
https://github.com/awf/autodiff/

#>

# Accept command-line args
param([string]$buildtype_="", 
		[int]$nruns_f=10, [int]$nruns_J=10, 
		[double]$time_limit=120, [double]$timeout=300, 
		[string]$tmpdir="", [switch]$repeat,
		[string[]]$tools=@())

# Assert function
function assert ($expr) {
    if (!(& $expr @args)) {
        throw "Assertion failed [$expr]"
    }
}

# Run command and (reliably) get output
function run_command ($indent, $outfile, $timeout, $cmd) {
	write-host "Run [$cmd $args]"
	$ProcessInfo = New-Object System.Diagnostics.ProcessStartInfo
	$ProcessInfo.FileName = $cmd
	$ProcessInfo.RedirectStandardError = $true
	$ProcessInfo.RedirectStandardOutput = $true
	$ProcessInfo.UseShellExecute = $false
	$ProcessInfo.Arguments = $args
	$Process = New-Object System.Diagnostics.Process
	$Process.StartInfo = $ProcessInfo
	try {
		$Process.Start() | Out-Null
	} catch {
		write-error "Failed to start process $cmd $args"
		throw "failed"
	}
	$status = $Process.WaitForExit($timeout * 1000)
	if ($status) {
		$stdout = $Process.StandardOutput.ReadToEnd().Trim().Replace("`n", "`n${indent}stdout> ")
		$stderr = $Process.StandardError.ReadToEnd().Trim().Replace("`n", "`n${indent}stderr> ")
		$allOutput = "${indent}stdout> " + $stdout + "`n${indent}stderr> " + $stderr
		Write-Host "$allOutput"
	} else {
		$Process.Kill()
		Write-Host "${indent}Killed after $timeout seconds"
		$result = "inf inf`ntf tJ`nFile generated by run-all.ps1 upon $timeout second timeout"
		Add-Content $outfile $result
	}
}

# Get source dir
$dir = split-path ($MyInvocation.MyCommand.Path)
assert { $dir -match 'ADbench$' }
$dir = Split-Path $dir

Write-Host "Root Directory: $dir"

# Load cmake variables
if ($buildtype_) { $buildtype = $buildtype_ }
else {
	if (Test-Path "$dir/ADBench/cmake-vars-Release.ps1") { $buildtype = "Release" }
	elseif (Test-Path "$dir/ADBench/cmake-vars-Debug.ps1") { $buildtype = "Debug" }
	else { throw "No cmake-vars file found. Remember to run cmake before running this script." }
}
assert Test-Path "$dir/ADBench/cmake-vars-$buildtype.ps1"
. $dir/ADBench/cmake-vars-$buildtype.ps1

# Set tmpdir default
if (!$tmpdir) { $tmpdir = "$dir/tmp" }
$tmpdir += "/$buildtype"

$datadir = "$dir/data"

Write-Host "Build Type: $buildtype`n"


# Custom Tool class
Class Tool {
	[string]$name
	[string]$type
	[array]$objectives
	[bool]$gmm_both
	[bool]$gmm_use_defs
	[array]$eigen_config

	# Static constants
	static [string]$gmm_dir_in = "$datadir/gmm/"
	static [string]$ba_dir_in = "$datadir/ba/"
	static [string]$hand_dir_in = "$datadir/hand/"
	static [string]$lstm_dir_in = "$datadir/lstm/"
	static [array]$gmm_sizes = @("1k", "10k") # also "2.5M"
	static [array]$hand_sizes = @("small", "big")
	static [int]$ba_min_n = 1
	static [int]$ba_max_n = 5
	static [int]$hand_min_n = 1
	static [int]$hand_max_n = 5
	static [array]$lstm_l_vals = @(2, 4)
	static [array]$lstm_c_vals = @(1024, 4096)
	# TODO probably want to set these in CMake somewhere

	# Constructor
	Tool ([string]$name, [string]$type, [string]$objectives, [bool]$gmm_both, [bool]$gmm_use_defs, [string]$eigen_config) {
		<#
		.SYNOPSIS
		Create a new Tool object to be run

		.EXAMPLE
		[Tool]::new("Finite", "bin", "1111", 0, 0, "101011")
		This will create a Tool:
		- called "Finite"
		- run from binary executables
		- runs all four tests 
		- does not do GMM in separate FULL and SPLIT modes
		- doesn't require separate executables for different GMM sizes
		- Runs all tests without eigen, and Hand tests with Eigen

		.NOTES
		$objectives is a binary string used to represent a boolean array, 
		where each element determines whether to run a certain objective:
		GMM, BA, HAND, LSTM
		
		$eigen_config works similarly, but now each pair of elements 
		determines firstly whether to run the objective with eigen, and 
		secondly whether to run it without.
		#>

		$this.name = $name
		$this.type = $type
		$this.objectives = $objectives.ToCharArray() | % { $_ -band "1" }
		$this.gmm_both = $gmm_both
		$this.gmm_use_defs = $gmm_use_defs
		$this.eigen_config = $eigen_config.ToCharArray() | % { $_ -band "1" }
	}

	# Run all tests for this tool
	[void] runall () {
		Write-Host $this.name
		if ($this.objectives[0]) { $this.testgmm() }
		if ($this.objectives[1]) { $this.testba() }
		if ($this.objectives[2]) { $this.testhand() }
		if ($this.objectives[3]) { $this.testlstm() }
	}

	# Run a single test
	[void] run ([string]$objective, [string]$dir_in, [string]$dir_out, [string]$fn) {
		if ($objective.contains("Eigen")) { $out_name = "$($this.name.ToLower())_eigen" }
		elseif ($objective.contains("Light")) { $out_name = "$($this.name.ToLower())_light" }
		elseif ($objective.endswith("SPLIT")) { $out_name = "$($this.name)_split" }
		else { $out_name = $this.name }
		$output_file = "${dir_out}${fn}_times_${out_name}.txt"
		if (!$script:repeat -and (Test-Path $output_file)) {
			Write-Host "          Skipped test (already completed)"
			return
		}

		$cmd = ""
		$cmdargs = @($dir_in, $dir_out, $fn, $script:nruns_f, $script:nruns_J, $script:time_limit)
		if ($this.type -eq "bin") {
			if ($this.name -eq "DiffSharp") {
				$builddir = ""
			} else {
				$builddir = "\$script:buildtype"
			}
			$cmd = "$script:bindir\tools\$($this.name)$builddir\Tools-$($this.name)-$objective.exe"

		} elseif ($this.type -eq "py" -or $this.type -eq "pybat") {
			$objective = $objective.ToLower().Replace("-", "_")
			if ($this.type -eq "py") { $cmd = "python" }
			elseif ($this.type -eq "pybat") { $cmd = "$script:dir/tools/$($this.name)/run.bat" }
			$cmdargs = @("$script:dir/tools/$($this.name)/$($this.name)_$objective.py") + $cmdargs

		} elseif ($this.type -eq "julia") {
			$objective = $objective.ToLower().Replace("-", "_")
			$cmd = "julia"
			$cmdargs = @("$script:dir/tools/$($this.name)/${objective}_F.jl") + $cmdargs

		} elseif ($this.type -eq "matlab") {
			$objective = $objective.ToLower().Replace("-", "_")
			$cmd = "matlab"
			$cmdargs = @("-wait", "-nosplash", "-nodesktop", "-r", "cd '$script:dir/tools/$($this.name)/'; addpath('$script:bindir/tools/$($this.name)/'); $($this.name)_$objective $cmdargs; quit")
		}

		run_command "          " $output_file $script:timeout $cmd @cmdargs
		if (!(test-path $output_file)) {
			throw "Command ran, but did not produce output file [$output_file]"
		}
	}

	# Run all gmm tests for this tool
	[void] testgmm () {
		$objs = @()
		if ($this.eigen_config[0]) { $objs += @("GMM") }
		if ($this.eigen_config[1]) { $objs += @("GMM-Eigen") }

		if ($this.gmm_both) { $types = @("-FULL", "-SPLIT") }
		else { $types = @("") }

		foreach ($obj in $objs) {
			foreach ($type in $types) {
				Write-Host "  $obj$type"

				foreach ($sz in [Tool]::gmm_sizes) {
					Write-Host "    $sz"

					$dir_in = "$([Tool]::gmm_dir_in)$sz/"
					$dir_out = "$script:tmpdir/gmm/$sz/$($this.name)/"
					mkdir -force $dir_out

					foreach ($d in $script:gmm_d_vals) {
						Write-Host "      d=$d"
						foreach ($k in $script:gmm_k_vals) {
							Write-Host "        K=$k"
							$run_obj = "$obj$type"
							if ($this.gmm_use_defs) { $run_obj += "-d$d-K$k" }
							$this.run($run_obj, $dir_in, $dir_out, "gmm_d${d}_K${k}")
						}
					}
				}
			}
		}
	}

	# Run all BA tests for this tool
	[void] testba () {
		$objs = @()
		if ($this.eigen_config[2]) { $objs += @("BA") }
		if ($this.eigen_config[3]) {$objs += @("BA-Eigen") }

		$dir_out = "$script:tmpdir/ba/$($this.name)/"
		if (!(Test-Path $dir_out)) { mkdir $dir_out }

		foreach ($obj in $objs) {
			Write-Host "  $obj"

			for ($n = [Tool]::ba_min_n; $n -le [Tool]::ba_max_n; $n++) {
				Write-Host "    $n"
				$this.run("$obj", [Tool]::ba_dir_in, $dir_out, "ba$n")
			}
		}
	}

	# Run all Hand tests for this tool
	[void] testhand () {
		$objs = @()
		if ($this.eigen_config[4]) {
			if ($this.type -eq "bin") { $objs += @("Hand-Light") }
			else { $objs += @("Hand") }
		}
		if ($this.eigen_config[5]) { $objs += @("Hand-Eigen") }

		foreach ($obj in $objs) {
			Write-Host "  $obj"

			foreach ($type in @("simple", "complicated")) {
				foreach ($sz in [Tool]::hand_sizes) {
					Write-Host "    ${type}_$sz"

					$dir_in = "$([Tool]::hand_dir_in)${type}_$sz/"
					$dir_out = "$script:tmpdir/hand/${type}_$sz/$($this.name)/"
					mkdir -force $dir_out

					for ($n = [Tool]::hand_min_n; $n -le [Tool]::hand_max_n; $n++) {
						Write-Host "      $n"
						$this.run("$obj-${type}", $dir_in, $dir_out, "hand$n")
					}
				}
			}
		}
	}

	[void] testlstm () {
		Write-Host "  LSTM"
		$dir_out = "$script:tmpdir/lstm/$($this.name)/"
		mkdir -force $dir_out

		foreach ($l in [Tool]::lstm_l_vals) {
			Write-Host "    l=$l"
			foreach ($c in [Tool]::lstm_c_vals) {
				Write-Host "      c=$c"

				$this.run("lstm", [Tool]::lstm_dir_in, $dir_out, "lstm_l${l}_c$c")
			}
		}
	}
}

# Full list of tool_descriptors
# Name
# runtype
# GMM, BA, HAND, LSTM
# Separate Full|Split?
# Separate GMM sizes?
$tool_descriptors = @(
	#[Tool]::new("Adept", "bin", "1110", 1, 0, "101010")
	#[Tool]::new("ADOLC", "bin", "1110", 1, 0, "101011")
	#[Tool]::new("Ceres", "bin", "1100", 0, 1, "101011")
	 [Tool]::new("Finite", "bin", "1111", 0, 0, "101011")
	 [Tool]::new("Manual", "bin", "1110", 0, 0, "110101")
	 [Tool]::new("DiffSharp", "bin", "0100", 1, 0, "101010")
	 [Tool]::new("Autograd", "py", "1100", 1, 0, "101010")
	 [Tool]::new("PyTorch", "py", "1011", 0, 0, "101010")
	 [Tool]::new("Julia", "julia", "1100", 0, 0, "101010")
	#[Tool]::new("Theano", "pybat", "1110", 0, 0, "101010")
	#[Tool]::new("MuPad", "matlab", 0, 0, 0)
	#[Tool]::new("ADiMat", "matlab", 0, 0, 0)
)

if ($tools) {
	foreach($tool in $tools) {
		write-host "User-specified tool $tool"
		$tool_descriptor = $tool_descriptors | ? { $_.name -eq $tool }
		if (!$tool_descriptor) {
			throw "Unknown tool [$tool]"
		}
		$tool_descriptor.runall()
	}
}
else {
	# Run all tests on each tool
	foreach ($tool_descriptor in $tool_descriptors) {
		$tool_descriptor.runall()
	}
}

# # Run Debug build configuration
# if (!$buildtype_ -and $buildtype -eq "Release") {
# 	if (Test-Path "$dir/ADBench/cmake-vars-Debug.ps1") {
# 		Write-Host "`n`n"

# 		$cmd = $MyInvocation.MyCommand.Path
# 		$cmdargs = @("Debug", $nruns_f, $nruns_J, $time_limit, $timeout, "", $repeat)
# 		& $cmd @cmdargs
# 	}
# }
