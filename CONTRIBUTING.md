# Contributing to Puma

Thank you for your interest in contributing to Puma! This document provides guidelines and best practices for contributing to the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Process](#development-process)
- [Coding Standards](#coding-standards)
- [Security Guidelines](#security-guidelines)
- [Testing](#testing)
- [Documentation](#documentation)
- [Submitting Changes](#submitting-changes)
- [Release Process](#release-process)

## Code of Conduct

We are committed to providing a welcoming and inclusive environment for all contributors. Please:

- Be respectful and considerate in your interactions
- Accept constructive criticism gracefully
- Focus on what is best for the community
- Show empathy towards other community members

## Getting Started

### Prerequisites

Before contributing, ensure you have:

1. **Development Environment**:
   - Qt 5.x or Qt 6.x
   - CMake 3.26 or higher
   - C++ compiler (Visual C++ 2015+ for Windows, GCC/Clang for Linux)
   - OpenSSL libraries
   - ImtCore framework (contact ImagingTools for access)

2. **Knowledge**:
   - C++ programming
   - Qt framework basics
   - CMake build system
   - Git version control

3. **Documentation**:
   - Read the [README.md](README.md)
   - Review [SECURITY.md](SECURITY.md) for security guidelines
   - Check [Docs/Dependencies.md](Docs/Dependencies.md) for dependency information

### Setting Up Your Development Environment

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/ImagingTools/Puma.git
   cd Puma
   ```

2. **Install Dependencies**:
   - Install Qt from https://www.qt.io/download
   - Install OpenSSL (system packages or from https://www.openssl.org/)
   - Set up ImtCore framework according to ImagingTools documentation

3. **Build the Project**:
   ```bash
   cd Build/CMake
   cmake ..
   cmake --build . --config Release
   ```

4. **Run Tests** (Windows only):
   ```bash
   # Enable testing
   cmake -DBUILD_TESTING=ON ..
   cmake --build . --config Release
   ctest
   ```

## Development Process

### Branching Strategy

- `main` or `master` - Stable release branch
- `develop` - Integration branch for features
- `feature/*` - Feature development branches
- `bugfix/*` - Bug fix branches
- `hotfix/*` - Critical security or bug fixes

### Workflow

1. **Create an Issue**:
   - Search for existing issues first
   - Create a new issue describing the bug or feature
   - Wait for discussion and approval for major changes

2. **Create a Branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Make Changes**:
   - Write clean, maintainable code
   - Follow coding standards (see below)
   - Add tests for new functionality
   - Update documentation as needed

4. **Test Your Changes**:
   - Build the project successfully
   - Run existing tests
   - Add new tests for your changes
   - Test on all supported platforms if possible

5. **Commit Your Changes**:
   ```bash
   git add .
   git commit -m "feat: Add feature description"
   ```

6. **Push and Create Pull Request**:
   ```bash
   git push origin feature/your-feature-name
   ```
   - Create a pull request on GitHub
   - Provide a clear description of changes
   - Reference related issues

## Coding Standards

### C++ Style Guidelines

- **Naming Conventions**:
  - Classes: `PascalCase` (e.g., `CAuthorizationController`)
  - Functions/Methods: `camelCase` or `PascalCase` (follow existing patterns)
  - Variables: `camelCase`
  - Constants: `UPPER_SNAKE_CASE`
  - Member variables: Prefix with `m_` (e.g., `m_userName`)

- **Formatting**:
  - Indentation: Tabs or spaces (be consistent with existing code)
  - Braces: Opening brace on same line for functions
  - Line length: Prefer 120 characters maximum

- **Best Practices**:
  - Use RAII (Resource Acquisition Is Initialization)
  - Prefer `nullptr` over `NULL`
  - Use smart pointers where appropriate
  - Avoid raw pointers when possible
  - Use `const` where appropriate
  - Initialize all variables

### Qt-Specific Guidelines

- Use Qt naming conventions for Qt classes
- Prefer Qt containers over STL when working with Qt APIs
- Use Qt signals/slots for event handling
- Follow Qt parent-child ownership model
- Use `Q_OBJECT` macro for classes with signals/slots

### Documentation

- Use Doxygen-style comments for public APIs:
  ```cpp
  /**
   * @brief Brief description
   * @param paramName Parameter description
   * @return Return value description
   * @note Important notes
   * @warning Security warnings
   */
  ```

- Document all public classes, methods, and functions
- Include usage examples for complex APIs
- Update documentation when changing behavior

## Security Guidelines

### Security is Critical

Puma handles authentication and authorization, so security is paramount:

1. **Never Commit Secrets**:
   - No passwords, API keys, or certificates in code
   - Use configuration files (excluded from git)
   - Use environment variables for sensitive data

2. **Input Validation**:
   - Validate all user inputs
   - Sanitize data before database operations
   - Use parameterized queries for SQL
   - Validate WebSocket messages

3. **Authentication & Authorization**:
   - Never bypass authentication checks
   - Use strong password hashing (bcrypt, Argon2)
   - Implement rate limiting for login attempts
   - Log security-relevant events

4. **Cryptography**:
   - Use OpenSSL for all cryptographic operations
   - Never implement custom crypto algorithms
   - Use strong cipher suites
   - Disable weak protocols (SSLv3, TLS 1.0, TLS 1.1)

5. **Dependencies**:
   - Keep all dependencies updated
   - Monitor security advisories
   - Document dependency versions
   - Use dependency scanning tools

6. **Code Review**:
   - All security-related code requires review
   - Use static analysis tools
   - Consider security implications of changes

### Reporting Security Vulnerabilities

**DO NOT** create public issues for security vulnerabilities. Instead:

- Read [SECURITY.md](SECURITY.md)
- Email security@imagingtools.org
- Follow coordinated disclosure practices

## Testing

### Test Requirements

- All new features must include tests
- Bug fixes should include regression tests
- Tests must pass before merging

### Testing Framework

- Use the `pumatest` framework for unit tests
- Test cases should be in appropriate test files
- Follow existing test patterns

### Running Tests

```bash
# Build with testing enabled
cmake -DBUILD_TESTING=ON ..
cmake --build . --config Release

# Run tests
ctest --verbose
```

### Test Coverage

- Aim for high test coverage on new code
- Focus on critical paths and edge cases
- Test error handling and boundary conditions

## Documentation

### What to Document

1. **Code Documentation**:
   - All public APIs
   - Complex algorithms
   - Security-relevant code
   - Configuration options

2. **User Documentation**:
   - Update README.md for user-visible changes
   - Update SDK documentation (AuthClientSdk.md, AuthServerSdk.md)
   - Add examples for new features

3. **Developer Documentation**:
   - Architecture decisions
   - Build instructions
   - Troubleshooting guides

### Generating API Documentation

```bash
cd Docs
doxygen Doxyfile
```

## Submitting Changes

### Pull Request Guidelines

1. **Before Submitting**:
   - Ensure all tests pass
   - Update documentation
   - Follow coding standards
   - Rebase on latest main/develop branch

2. **PR Description**:
   - Clear title describing the change
   - Detailed description of what and why
   - Reference related issues
   - List any breaking changes
   - Include screenshots for UI changes

3. **PR Review**:
   - Be responsive to feedback
   - Make requested changes promptly
   - Keep discussions professional
   - Be patient during review process

4. **Commit Messages**:
   - Use conventional commit format:
     - `feat:` New feature
     - `fix:` Bug fix
     - `docs:` Documentation changes
     - `test:` Test additions/changes
     - `refactor:` Code refactoring
     - `security:` Security fixes
   - Example: `feat: Add multi-factor authentication support`

### Review Process

1. Automated checks run (build, tests, linting)
2. Code review by maintainers
3. Security review for security-related changes
4. Testing on multiple platforms (if needed)
5. Approval and merge

## Release Process

### Version Management

Puma uses Git-based version management:

- See [Build/Git/README.md](Build/Git/README.md) for details
- Version numbers follow semantic versioning (MAJOR.MINOR.PATCH)
- Security fixes increment PATCH version
- New features increment MINOR version
- Breaking changes increment MAJOR version

### Release Checklist

1. Update CHANGELOG.md
2. Update version numbers
3. Run full test suite
4. Update documentation
5. Update SBOM (Docs/SBOM.json) if dependencies changed
6. Create release tag
7. Build release artifacts
8. Publish release notes

### Security Releases

For security fixes:

1. Follow [SECURITY.md](SECURITY.md) disclosure policy
2. Prepare fix in private branch
3. Coordinate with security researchers
4. Release patch with security advisory
5. Notify users through all channels

## EU CRA Compliance

When contributing, help maintain EU Cyber Resilience Act compliance:

1. **Document Dependencies**:
   - Update [Docs/Dependencies.md](Docs/Dependencies.md) for new dependencies
   - Update [Docs/SBOM.json](Docs/SBOM.json) with dependency information

2. **Security Considerations**:
   - Follow security guidelines above
   - Document security implications of changes
   - Participate in security reviews

3. **Vulnerability Management**:
   - Report vulnerabilities through proper channels
   - Monitor dependency security advisories
   - Apply security patches promptly

## Additional Resources

- **Qt Documentation**: https://doc.qt.io/
- **CMake Documentation**: https://cmake.org/documentation/
- **OpenSSL Documentation**: https://www.openssl.org/docs/
- **C++ Core Guidelines**: https://isocpp.github.io/CppCoreGuidelines/

## Getting Help

- **Questions**: Open a GitHub issue with the "question" label
- **Chat**: Join our community chat (if available)
- **Email**: Contact maintainers through GitHub

## Recognition

Contributors will be recognized in:
- CHANGELOG.md
- Release notes
- Contributors list (if maintained)

Thank you for contributing to Puma!
