#==============================================================================
# Copyright Intel Corporation
#
# SPDX-License-Identifier: MIT
#==============================================================================
"""Surface Sharing API Validation Script."""
import subprocess  # nosec
import glob
import os
from enum import Enum


class SurfaceComponent(Enum):
    """Surface component list"""
    ENCODE = "encode"
    DECODE = "decode"
    VPP_INPUT = "vpp_in"
    VPP_OUTPUT = "vpp_out"


class SurfaceType(Enum):
    """Surface type list"""
    D3D11_TEX2D = "d3d11"
    VAAPI = "vaapi"
    OPENCL_IMG2D = "opencl"


class Mode(Enum):
    """Mode list"""
    COPY = "copy"
    SHARED = "shared"


# pylint: disable=too-few-public-methods
class TestStats:
    """Test staticstics for the report"""
    def __init__(self):
        self.n_total_testcase = 0
        self.n_total_failed = 0
        self.n_total_pass = 0


# pylint: disable=too-few-public-methods
class TestVariablesToShare:
    """Test variables to share"""
    def __init__(self):
        self.multi_session_test = False
        self.out_filename = ""


# pylint: disable=too-many-instance-attributes
class TestCategory:
    """Test category"""
    def __init__(self):
        self.surface_component = ""
        self.surface_type = ""
        self.mode = ""
        self.category = ""
        self.case = ""

        self.n_failed = 0
        self.n_pass = 0
        self.n_testcase = 0

    def init_values(self):
        """Init values"""
        self.surface_component = ""
        self.surface_type = ""
        self.mode = ""
        self.category = ""
        self.case = ""

        self.n_failed = 0
        self.n_pass = 0
        self.n_testcase = 0

    def get_category_cli_options(self):
        """Build command line and return it"""
        cmd = []
        if self.surface_component:
            cmd.extend(["-surface_component", self.surface_component])
        if self.surface_type:
            cmd.extend(["-surface_type", self.surface_type])
        if self.mode:
            cmd.extend(["-mode", self.mode])

        return cmd

    def get_category_name(self):
        """Return category name"""
        return self.category

    def set_category(self,
                     surface_component: SurfaceComponent,
                     surface_type: SurfaceType = None,
                     mode: Mode = None):
        """Set test category"""
        self.init_values()

        self.category = self.surface_component = (surface_component).value

        if surface_type:
            self.surface_type = (surface_type).value
            self.category += ", " + self.surface_type
        else:
            self.surface_type = ""

        if mode:
            self.mode = (mode).value
            self.category += ", " + self.mode
        else:
            self.mode = ""

    def report_result(self, test_stats: TestStats):
        """Summary test result of this category"""
        self.n_testcase = self.n_failed + self.n_pass

        # this is not related to password at all but means "test passed"
        pass_or_fail = "PASS"  # nosec
        if self.n_failed:
            pass_or_fail = "FAILED"  # nosec

        result_detail = '  ' + str(self.n_testcase) + " tests, " + str(
            self.n_pass) + " passed, " + str(self.n_failed) + " failed"

        out_space = ' ' * 74
        left_space = len(out_space) - len(
            self.category) - len(pass_or_fail) - 4
        result = '  ' + self.category + ' ' * left_space + pass_or_fail

        print(result)
        print(result_detail)
        print(
            "=========================================================================="
        )

        test_stats.n_total_testcase += self.n_testcase
        test_stats.n_total_failed += self.n_failed
        test_stats.n_total_pass += self.n_pass


# global declarations

# test information
TC = TestCategory()
# test statistics
TS = TestStats()
# test variables to share
TV = TestVariablesToShare()

VPL_EXAMPLES_PATH = os.environ.get('VPL_EXAMPLES_PATH')
if VPL_EXAMPLES_PATH:
    INPUT_CONTENT = f"{VPL_EXAMPLES_PATH}/content"
    VAL_APP = r"val-surface-sharing"
    print(INPUT_CONTENT)
else:
    raise SystemExit("environment is not ready!")


def display_adapters():
    """Display available adapters information"""
    # build command
    cmd = [VAL_APP, "-adapters"]

    print("\n[Command] " + ' '.join(cmd) + "\n")

    with subprocess.Popen(cmd, shell=False) as proc:  # nosec
        proc.communicate()
        if proc.returncode != 0:
            raise Exception("Error running command: " + cmd)
        return proc.returncode


# pylint: disable=invalid-name, too-many-arguments
def run_test(c=None,
             i=None,
             o=None,
             sw=None,
             sh=None,
             sc=None,
             dw=None,
             dh=None,
             dc=None,
             multi_session=False):
    """Compose a test command line and execute it"""
    # init
    TV.multi_session_test = False
    TV.out_filename = ""
    TC.case = ""

    # build command
    cmd = [VAL_APP]

    # get composed the command line for surface component, surface type, mode
    cmd.extend(TC.get_category_cli_options())
    TC.case += TC.get_category_name()

    # set from function arguments
    if c:
        cmd.extend(["-c", c])
        TC.case += "/" + c
    if i:
        cmd.extend(["-i", f"{INPUT_CONTENT}/{i}"])
    if o:
        cmd.extend(["-o", o])
        TV.out_filename = o
    if sw and sw and sc:
        cmd.extend(["-sw", str(sw), "-sh", str(sh), "-sc", sc])
        TC.case += "/" + str(sw) + "x" + str(sh) + "," + sc
    if dw and dw and dc:
        cmd.extend(["-dw", str(dw), "-dh", str(dh), "-dc", dc])
        TC.case += "->" + str(dw) + "x" + str(dh) + "," + dc
    if multi_session:
        cmd.extend(["-multi_session"])
        TV.multi_session_test = True
        TC.case += "/" + "multi_session"

    print("\n[Command] " + ' '.join(cmd) + "\n")

    with subprocess.Popen(cmd, shell=False) as proc:  # nosec
        proc.communicate()
        return proc.returncode


def print_result(pass_or_fail):
    """Print result"""
    out_space = ' ' * 74
    left_space = len(out_space) - len(TC.case) - len(pass_or_fail) - 4
    result = '  ' + TC.case + ' ' * left_space + pass_or_fail
    print(
        "--------------------------------------------------------------------------"
    )
    print(result)
    print(
        "--------------------------------------------------------------------------"
    )


def check_output_multi_session(ref):
    """Compare the output contents from mult-session test with the reference content"""
    if not os.path.exists(ref) or os.path.getsize(ref) == 0:
        print("\n  Error: check_output_multi_session(): " + ref +
              " does not exist or size is 0")
        return False

    # there'll be always 2 output files at least with {filename}.1, 2 .. {filename}.n
    # but, it's enough to check only 1st and 2nd
    for i in range(2):
        m_out_filename = f"{TV.out_filename}.{i+1}"

        if not os.path.exists(m_out_filename) or os.path.getsize(
                m_out_filename) == 0:
            print("\n  Error: check_output_multi_session(): " +
                  m_out_filename + " does not exist or size is 0")
            return False

        block_size = 100 * 1024
        with open(ref, "rb") as fref, open(m_out_filename, "rb") as fout:
            while True:
                block_ref = fref.read(block_size)
                block_out = fout.read(block_size)
                if block_ref != block_out:
                    print(
                        "\n  Error: check_output_multi_session(): outputs are different!!"
                    )
                    return False
                if not block_ref and not block_out:
                    return True


def check_output(ref):
    """Compare the output content from the regular test with the reference content"""
    if not os.path.exists(ref) or os.path.getsize(ref) == 0:
        print("\n  Error: check_output(): " + ref +
              " does not exist or size is 0")
        return False

    if not os.path.exists(TV.out_filename) or os.path.getsize(
            TV.out_filename) == 0:
        print("\n  Error: check_output(): " + TV.out_filename +
              " does not exist")
        return False

    block_size = 100 * 1024
    with open(ref, "rb") as fref, open(TV.out_filename, "rb") as fout:
        while True:
            block_ref = fref.read(block_size)
            block_out = fout.read(block_size)
            if block_ref != block_out:
                print("\n  Error: check_output(): outputs are different!!")
                return False
            if not block_ref and not block_out:
                return True


def handle_result(returncode, ref=None):
    """Handle result"""
    if returncode != 0:
        TC.n_failed += 1
        print_result("FAILED")
    else:
        if ref is None:
            TC.n_pass += 1
            print_result("PASS")
        else:
            res = False

            if TV.multi_session_test is True:
                res = check_output_multi_session(ref)
            else:
                res = check_output(ref)

            if res is False:
                print_result("FAILED")
                TC.n_failed += 1
            else:
                print_result("PASS")
                TC.n_pass += 1


def print_test(stest):
    """Print 'stest'"""
    print(
        "\n=========================================================================="
    )
    print(stest)
    print(
        "=========================================================================="
    )


def summary_final():
    """Summary final report"""
    test_final = "Surface Sharing API functional Validation Test"

    # this is not related to password at all but means "test passed"
    pass_or_fail = "PASS"  # nosec
    if TS.n_total_failed:
        pass_or_fail = "FAILED"  # nosec

    result_final = '  ' + str(TS.n_total_testcase) + " tests, " + str(
        TS.n_total_pass) + " passed, " + str(TS.n_total_failed) + " failed"

    out_space = ' ' * 74
    left_space = len(out_space) - len(test_final) - len(pass_or_fail) - 4
    result = '  ' + test_final + ' ' * left_space + pass_or_fail
    print('  ' + "Result Final:")
    print(result)
    print(result_final)
    print(
        "=========================================================================="
    )


def clean_up_outputs():
    """Clean up temporary output contents after each test"""
    files_to_delete = ["out.*", "ref_*"]
    files_found = [
        filename for pattern in files_to_delete
        for filename in glob.glob(pattern)
    ]
    if not files_found:
        print("No, 'out.*' or 'ref*' files found")
    else:
        for filename in files_found:
            try:
                os.remove(filename)
            # pylint: disable=broad-except
            except Exception as err_info:
                print(f"Error deleting {filename}: {err_info}")


# pylint: disable=too-many-statements
def run_surface_type_test(surface_type: SurfaceType):
    """List DX11 test cases and run"""
    print_test("[Test] Encode Component")

    TC.set_category(SurfaceComponent.ENCODE)

    # no surface sharing api
    # prepare reference stream to compare
    handle_result(
        run_test(c="h264",
                 i="cars_320x240.nv12",
                 sw=320,
                 sh=240,
                 sc="nv12",
                 o="ref_nv12.h264"))
    handle_result(
        run_test(c="h264",
                 i="cars_320x240.bgra",
                 sw=320,
                 sh=240,
                 sc="rgb4",
                 o="ref_rgb4.h264"))
    handle_result(
        run_test(c="h265",
                 i="cars_320x240.nv12",
                 sw=320,
                 sh=240,
                 sc="nv12",
                 o="ref_nv12.h265"))
    handle_result(
        run_test(c="h265",
                 i="cars_320x240.bgra",
                 sw=320,
                 sh=240,
                 sc="rgb4",
                 o="ref_rgb4.h265"))
    TC.report_result(TS)

    # disable opencl encode - import shared mode testing until gpu-rt supports
    if surface_type != SurfaceType.OPENCL_IMG2D:
        # run surface sharing test
        TC.set_category(SurfaceComponent.ENCODE, surface_type, Mode.SHARED)
        handle_result(run_test(c="h264",
                               i="cars_320x240.nv12",
                               sw=320,
                               sh=240,
                               sc="nv12",
                               o="out.h264"),
                      ref="ref_nv12.h264")
        handle_result(run_test(c="h264",
                               i="cars_320x240.bgra",
                               sw=320,
                               sh=240,
                               sc="rgb4",
                               o="out.h264"),
                      ref="ref_rgb4.h264")
        handle_result(run_test(c="h265",
                               i="cars_320x240.nv12",
                               sw=320,
                               sh=240,
                               sc="nv12",
                               o="out.h265"),
                      ref="ref_nv12.h265")
        handle_result(run_test(c="h265",
                               i="cars_320x240.bgra",
                               sw=320,
                               sh=240,
                               sc="rgb4",
                               o="out.h265"),
                      ref="ref_rgb4.h265")
        handle_result(run_test(c="h264",
                               i="cars_320x240.nv12",
                               sw=320,
                               sh=240,
                               sc="nv12",
                               o="out.h264",
                               multi_session=True),
                      ref="ref_nv12.h264")
        handle_result(run_test(c="h265",
                               i="cars_320x240.bgra",
                               sw=320,
                               sh=240,
                               sc="rgb4",
                               o="out.h265",
                               multi_session=True),
                      ref="ref_rgb4.h265")
        TC.report_result(TS)

    TC.set_category(SurfaceComponent.ENCODE, surface_type, Mode.COPY)
    handle_result(run_test(c="h264",
                           i="cars_320x240.nv12",
                           sw=320,
                           sh=240,
                           sc="nv12",
                           o="out.h264"),
                  ref="ref_nv12.h264")
    handle_result(run_test(c="h264",
                           i="cars_320x240.bgra",
                           sw=320,
                           sh=240,
                           sc="rgb4",
                           o="out.h264"),
                  ref="ref_rgb4.h264")
    handle_result(run_test(c="h265",
                           i="cars_320x240.nv12",
                           sw=320,
                           sh=240,
                           sc="nv12",
                           o="out.h265"),
                  ref="ref_nv12.h265")
    handle_result(run_test(c="h265",
                           i="cars_320x240.bgra",
                           sw=320,
                           sh=240,
                           sc="rgb4",
                           o="out.h265"),
                  ref="ref_rgb4.h265")
    handle_result(run_test(c="h264",
                           i="cars_320x240.nv12",
                           sw=320,
                           sh=240,
                           sc="nv12",
                           o="out.h264",
                           multi_session=True),
                  ref="ref_nv12.h264")
    handle_result(run_test(c="h265",
                           i="cars_320x240.bgra",
                           sw=320,
                           sh=240,
                           sc="rgb4",
                           o="out.h265",
                           multi_session=True),
                  ref="ref_rgb4.h265")
    TC.report_result(TS)
    clean_up_outputs()

    print_test("[Test] Decode Component")

    TC.set_category(SurfaceComponent.DECODE)

    # no surface sharing api
    # prepare reference stream to compare
    handle_result(run_test(c="h264", i="cars_320x240.h264", o="ref_h264.nv12"))
    handle_result(run_test(c="h265", i="cars_320x240.h265", o="ref_h265.nv12"))
    TC.report_result(TS)

    # run surface sharing test
    TC.set_category(SurfaceComponent.DECODE, surface_type, Mode.SHARED)
    handle_result(run_test(c="h264", i="cars_320x240.h264", o="out.nv12"),
                  ref="ref_h264.nv12")
    handle_result(run_test(c="h265", i="cars_320x240.h265", o="out.nv12"),
                  ref="ref_h265.nv12")
    handle_result(run_test(c="h264",
                           i="cars_320x240.h264",
                           o="out.nv12",
                           multi_session=True),
                  ref="ref_h264.nv12")
    TC.report_result(TS)

    TC.set_category(SurfaceComponent.DECODE, surface_type, Mode.COPY)
    handle_result(run_test(c="h264", i="cars_320x240.h264", o="out.nv12"),
                  ref="ref_h264.nv12")
    handle_result(run_test(c="h265", i="cars_320x240.h265", o="out.nv12"),
                  ref="ref_h265.nv12")
    handle_result(run_test(c="h265",
                           i="cars_320x240.h265",
                           o="out.nv12",
                           multi_session=True),
                  ref="ref_h265.nv12")
    TC.report_result(TS)
    clean_up_outputs()

    print_test("[Test] VPP In Component")

    TC.set_category(SurfaceComponent.VPP_INPUT)

    # no surface sharing api
    # prepare reference stream to compare
    handle_result(
        run_test(i="cars_320x240.nv12",
                 sw=320,
                 sh=240,
                 sc="nv12",
                 o="ref_nv12.bgra",
                 dw=352,
                 dh=288,
                 dc="rgb4"))
    handle_result(
        run_test(i="cars_320x240.bgra",
                 sw=320,
                 sh=240,
                 sc="rgb4",
                 o="ref_rgb4.nv12",
                 dw=352,
                 dh=288,
                 dc="nv12"))
    TC.report_result(TS)

    # disable opencl vpp_input - import shared mode testing until gpu-rt supports
    if surface_type != SurfaceType.OPENCL_IMG2D:
        # run surface sharing test
        TC.set_category(SurfaceComponent.VPP_INPUT, surface_type, Mode.SHARED)
        handle_result(run_test(i="cars_320x240.nv12",
                               sw=320,
                               sh=240,
                               sc="nv12",
                               o="out.bgra",
                               dw=352,
                               dh=288,
                               dc="rgb4"),
                      ref="ref_nv12.bgra")
        handle_result(run_test(i="cars_320x240.bgra",
                               sw=320,
                               sh=240,
                               sc="rgb4",
                               o="out.nv12",
                               dw=352,
                               dh=288,
                               dc="nv12"),
                      ref="ref_rgb4.nv12")
        handle_result(run_test(i="cars_320x240.nv12",
                               sw=320,
                               sh=240,
                               sc="nv12",
                               o="out.bgra",
                               dw=352,
                               dh=288,
                               dc="rgb4",
                               multi_session=True),
                      ref="ref_nv12.bgra")
        TC.report_result(TS)

    TC.set_category(SurfaceComponent.VPP_INPUT, surface_type, Mode.COPY)
    handle_result(run_test(i="cars_320x240.nv12",
                           sw=320,
                           sh=240,
                           sc="nv12",
                           o="out.bgra",
                           dw=352,
                           dh=288,
                           dc="rgb4"),
                  ref="ref_nv12.bgra")
    handle_result(run_test(i="cars_320x240.bgra",
                           sw=320,
                           sh=240,
                           sc="rgb4",
                           o="out.nv12",
                           dw=352,
                           dh=288,
                           dc="nv12"),
                  ref="ref_rgb4.nv12")
    handle_result(run_test(i="cars_320x240.bgra",
                           sw=320,
                           sh=240,
                           sc="rgb4",
                           o="out.nv12",
                           dw=352,
                           dh=288,
                           dc="nv12",
                           multi_session=True),
                  ref="ref_rgb4.nv12")
    TC.report_result(TS)
    clean_up_outputs()

    print_test("[Test] VPP Out Component")

    TC.set_category(SurfaceComponent.VPP_OUTPUT)

    # no surface sharing api
    # prepare reference stream to compare
    handle_result(
        run_test(i="cars_320x240.nv12",
                 sw=320,
                 sh=240,
                 sc="nv12",
                 o="ref_nv12.bgra",
                 dw=352,
                 dh=288,
                 dc="rgb4"))
    handle_result(
        run_test(i="cars_320x240.bgra",
                 sw=320,
                 sh=240,
                 sc="rgb4",
                 o="ref_rgb4.nv12",
                 dw=352,
                 dh=288,
                 dc="nv12"))
    TC.report_result(TS)

    # run surface sharing test
    TC.set_category(SurfaceComponent.VPP_OUTPUT, surface_type, Mode.SHARED)
    handle_result(run_test(i="cars_320x240.nv12",
                           sw=320,
                           sh=240,
                           sc="nv12",
                           o="out.bgra",
                           dw=352,
                           dh=288,
                           dc="rgb4"),
                  ref="ref_nv12.bgra")
    handle_result(run_test(i="cars_320x240.bgra",
                           sw=320,
                           sh=240,
                           sc="rgb4",
                           o="out.nv12",
                           dw=352,
                           dh=288,
                           dc="nv12"),
                  ref="ref_rgb4.nv12")
    handle_result(run_test(i="cars_320x240.nv12",
                           sw=320,
                           sh=240,
                           sc="nv12",
                           o="out.bgra",
                           dw=352,
                           dh=288,
                           dc="rgb4",
                           multi_session=True),
                  ref="ref_nv12.bgra")
    TC.report_result(TS)

    TC.set_category(SurfaceComponent.VPP_OUTPUT, surface_type, Mode.COPY)
    handle_result(run_test(i="cars_320x240.nv12",
                           sw=320,
                           sh=240,
                           sc="nv12",
                           o="out.bgra",
                           dw=352,
                           dh=288,
                           dc="rgb4"),
                  ref="ref_nv12.bgra")
    handle_result(run_test(i="cars_320x240.bgra",
                           sw=320,
                           sh=240,
                           sc="rgb4",
                           o="out.nv12",
                           dw=352,
                           dh=288,
                           dc="nv12"),
                  ref="ref_rgb4.nv12")
    handle_result(run_test(i="cars_320x240.bgra",
                           sw=320,
                           sh=240,
                           sc="rgb4",
                           o="out.nv12",
                           dw=352,
                           dh=288,
                           dc="nv12",
                           multi_session=True),
                  ref="ref_rgb4.nv12")
    TC.report_result(TS)
    clean_up_outputs()


def main():
    """Main"""
    print_test("[Info] Gen Adapters")
    display_adapters()

    if os.name == 'nt':
        # dx11 test
        run_surface_type_test(SurfaceType.D3D11_TEX2D)

        # opencl test
        run_surface_type_test(SurfaceType.OPENCL_IMG2D)
    else:
        # vaapi test
        run_surface_type_test(SurfaceType.VAAPI)

    # final report
    summary_final()


if __name__ == '__main__':
    main()
