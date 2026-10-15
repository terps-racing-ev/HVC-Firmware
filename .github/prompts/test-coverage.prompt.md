# 100% Test Coverage on Code Additions

## Description
Enforces a strict workflow to ensure all new code additions achieve 100% test coverage. This skill guides the agent to systematically identify new logic, write comprehensive tests, and ensure all branches and edge cases are verified.

## Trigger
Automatically apply this skill whenever new code is added, a feature is implemented, or a bug is fixed. You do not need to wait for the user to request tests.

## Instructions

When applying this skill, follow these steps strictly:

1. **Identify Code Additions & Check Necessity**
   - Review the diff or the newly added code to determine if test additions are necessary (e.g., skip if it's purely a documentation change).
   - Map out all new functions, methods, branching logic (if/else, switch), and error-handling paths.

2. **Clarify Intent (CRITICAL)**
   - If there are ANY ambiguities about the intended behavior of a function or code block, STOP.
   - You MUST ask the user for clarification 100% of the time before proceeding to write tests. Do NOT make assumptions.

3. **Locate or Initialize the Test Suite**
   - Find the corresponding test file (e.g., in the `tests/` directory matching the source file name `test_<module>.c`).
   - If no test file exists, create the boilerplate using the project's standard testing framework (e.g., Unity/CMock based on the workspace).

4. **Analyze Execution Paths**
   - List the "Happy Path" (expected standard input/output).
   - List all "Edge Cases" (boundary conditions, max/min values).
   - List all "Error Paths" (null pointers, timeout responses, invalid states).

5. **Implement Tests**
   - Write individual test cases for each identified path to guarantee 100% branch and line coverage on the new code.
   - Leverage mocks and stubs for any external dependencies (e.g., hardware abstraction layers, other managers).

6. **Verify Coverage via Docker**
   - Execute the test and coverage generation commands inside the project's testing Docker container (based on `Dockerfile.test`).
   - Be as efficient as possible with Docker usage (e.g., reuse the container if it's already built and running, run commands interactively or via `docker exec` if applicable, rather than rebuilding from scratch every time).
   - If the Docker daemon isn't running or there is a container setup issue, immediately prompt the user to start Docker or resolve the issue before continuing.
   - If the coverage report shows < 100% on the new lines, repeat steps 4-5 for the uncovered lines until 100% is achieved.
