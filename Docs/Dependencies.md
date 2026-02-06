# Puma Dependencies

This document provides a comprehensive overview of all third-party dependencies used in the Puma project, their purposes, licensing information, and security considerations.

## Overview

Puma relies on several external libraries and frameworks to provide its authentication and authorization functionality. Understanding these dependencies is crucial for:
- Security vulnerability management
- License compliance
- Building and deployment
- EU Cyber Resilience Act (CRA) compliance

## Core Runtime Dependencies

### Qt Framework (REQUIRED)

**Version**: Qt 5.x or Qt 6.x  
**License**: LGPL-3.0 / GPL-2.0 / GPL-3.0 / Commercial  
**Purpose**: Cross-platform application framework  
**Website**: https://www.qt.io/

**Modules Used**:
- `Qt::Core` - Core non-graphical classes
- `Qt::Widgets` - Widget-based UI components
- `Qt::QuickWidgets` - Embedding QML in widgets
- `Qt::Gui` - GUI base classes
- `Qt::Xml` - XML parsing
- `Qt::Network` - Network programming
- `Qt::Svg` - SVG rendering
- `Qt::Sql` - Database integration (SQLite)
- `Qt::WebSockets` - WebSocket protocol implementation
- `Qt::Qml` - QML engine
- `Qt::Concurrent` - Multi-threading support
- `Qt::Core5Compat` - Qt 5 compatibility (Qt 6 only)

**Security Considerations**:
- Keep Qt updated to receive security patches
- Monitor Qt security advisories: https://www.qt.io/product/security
- Qt WebSockets should always use secure connections (WSS) in production
- Qt SQL should use parameterized queries to prevent SQL injection

**CVE Monitoring**:
- Subscribe to Qt security mailing lists
- Check https://www.cvedetails.com/vendor/3836/Qt.html

---

### OpenSSL (REQUIRED)

**Version**: 1.1.1 or later (3.x recommended)  
**License**: Apache-2.0 (OpenSSL 3.x), OpenSSL License (1.x)  
**Purpose**: SSL/TLS encryption and cryptographic operations  
**Website**: https://www.openssl.org/

**Usage in Puma**:
- SSL/TLS connections for secure client-server communication
- Certificate validation and management
- Cryptographic operations for authentication

**Security Considerations**:
- **CRITICAL**: Always use OpenSSL 1.1.1 or later (1.0.x is end-of-life)
- OpenSSL 3.x is strongly recommended for new deployments
- Monitor OpenSSL security advisories: https://www.openssl.org/news/vulnerabilities.html
- Configure strong cipher suites
- Disable weak protocols (SSLv3, TLS 1.0, TLS 1.1)
- Keep OpenSSL updated with latest security patches

**Known Historical Vulnerabilities**:
- Heartbleed (CVE-2014-0160) - Fixed in 1.0.1g
- POODLE (CVE-2014-3566) - SSLv3 vulnerability
- DROWN (CVE-2016-0800) - SSLv2 vulnerability

**CVE Monitoring**:
- https://www.openssl.org/news/vulnerabilities.html
- https://www.cvedetails.com/vendor/217/Openssl.html

---

### SQLite (REQUIRED)

**Version**: 3.x (included with Qt)  
**License**: Public Domain  
**Purpose**: SQL database engine  
**Website**: https://www.sqlite.org/

**Usage in Puma**:
- User account storage
- Authorization data persistence
- Configuration storage
- Session management

**Security Considerations**:
- Use appropriate file permissions for database files
- Consider encryption at rest for sensitive data
- Use parameterized queries (provided by Qt SQL)
- Regularly backup database files
- Monitor SQLite security advisories: https://www.sqlite.org/security.html

**CVE Monitoring**:
- https://www.sqlite.org/security.html
- https://www.cvedetails.com/vendor/9237/Sqlite.html

---

### ImtCore Framework (REQUIRED)

**Version**: Various  
**License**: Proprietary / NOASSERTION  
**Purpose**: Component-based application framework  
**Source**: ImagingTools

**Components Used**:
- `ImtCoreVoce` - Core component framework
- `ImtAuthVoce` - Authentication components
- `ImtServerGqlPck` - GraphQL server implementation
- `ImtUserAdministrationVoce` - User administration
- Additional support packages (17+ components)

**Purpose in Puma**:
- Component architecture and lifecycle management
- Authentication infrastructure
- GraphQL API server
- User management functionality
- HTTP server capabilities

**Security Considerations**:
- Follow ImagingTools security guidelines
- Keep ImtCore components updated
- Monitor for security advisories from ImagingTools

---

## Build-Time Dependencies

### CMake (REQUIRED for building)

**Version**: 3.26 or higher  
**License**: BSD-3-Clause  
**Purpose**: Build system generator  
**Website**: https://cmake.org/

**Usage**:
- Primary build system for Puma
- Dependency management
- Cross-platform build configuration

**Note**: Build-time only, not included in runtime distribution

---

### C++ Compiler (REQUIRED for building)

**Supported Compilers**:
- **Windows**: Visual C++ 2015/2017/2019 or later
- **Linux**: GCC or Clang

**Standard**: C++11 or later

---

## Optional Dependencies

### QMake (OPTIONAL)

**Version**: Included with Qt  
**License**: Same as Qt  
**Purpose**: Alternative build system  

**Note**: QMake build system is supported as an alternative to CMake. See `Build/QMake/` directory.

---

## Dependency Management

### Updating Dependencies

To update dependencies:

1. **Qt**: Download and install from https://www.qt.io/download
2. **OpenSSL**: Update system OpenSSL packages or download from https://www.openssl.org/
3. **SQLite**: Typically updated with Qt; can be updated separately if needed
4. **ImtCore**: Follow ImagingTools update procedures

### Version Compatibility

| Puma Version | Qt Version | OpenSSL Version | CMake Version | ImtCore Version |
|--------------|------------|-----------------|---------------|-----------------|
| Latest       | 5.x / 6.x  | 1.1.1+ / 3.x   | 3.26+         | Latest          |

### Checking Installed Versions

```bash
# Qt version
qmake --version

# OpenSSL version
openssl version

# CMake version
cmake --version
```

---

## Security Vulnerability Monitoring

### Recommended Practices

1. **Subscribe to Security Advisories**:
   - Qt: https://www.qt.io/product/security
   - OpenSSL: https://www.openssl.org/news/vulnerabilities.html
   - SQLite: https://www.sqlite.org/security.html

2. **Use Automated Scanning**:
   - Dependabot (GitHub)
   - Snyk (https://snyk.io/)
   - OWASP Dependency-Check (https://owasp.org/www-project-dependency-check/)

3. **Regular Updates**:
   - Check for updates monthly
   - Apply security patches immediately
   - Test updates in non-production environment first

4. **Monitor CVE Databases**:
   - National Vulnerability Database (NVD): https://nvd.nist.gov/
   - CVE Details: https://www.cvedetails.com/

### Known Vulnerability Resources

- **Qt CVEs**: https://www.cvedetails.com/vendor/3836/Qt.html
- **OpenSSL CVEs**: https://www.cvedetails.com/vendor/217/Openssl.html
- **SQLite CVEs**: https://www.cvedetails.com/vendor/9237/Sqlite.html

---

## License Compliance

### Open Source Licenses Used

| Dependency | License | Type | Commercial Use | Modifications |
|------------|---------|------|----------------|---------------|
| Qt         | LGPL-3.0 / GPL / Commercial | Copyleft / Commercial | ✓ (with compliance) | ✓ |
| OpenSSL    | Apache-2.0 | Permissive | ✓ | ✓ |
| SQLite     | Public Domain | Public Domain | ✓ | ✓ |
| CMake      | BSD-3-Clause | Permissive | ✓ | ✓ |
| ImtCore    | Proprietary | Proprietary | ? | ? |

### License Compliance Notes

**Qt LGPL Compliance**:
- If using LGPL Qt, you must allow users to replace Qt libraries
- Alternatively, use commercial Qt license for proprietary applications
- See: https://www.qt.io/licensing/

**OpenSSL**:
- Apache 2.0 license (3.x) is permissive and allows commercial use
- Older OpenSSL license (1.x) has special advertising clause
- Attribution required in documentation

**SQLite**:
- Public domain - no license restrictions
- No attribution required but appreciated

---

## Supply Chain Security

### Dependency Verification

When building Puma:

1. **Download from Official Sources**:
   - Qt: https://www.qt.io/download
   - OpenSSL: https://www.openssl.org/source/
   - CMake: https://cmake.org/download/

2. **Verify Checksums**:
   - Always verify SHA256 checksums of downloaded packages
   - Use GPG signatures when available

3. **Use Package Managers** (when possible):
   - Linux: Use system package managers (apt, yum, etc.)
   - Windows: Use vcpkg or official installers
   - Ensure package manager repositories are trusted

### Build Reproducibility

To ensure reproducible builds:
- Document exact versions of all dependencies
- Use version pinning in build scripts
- Test builds in clean environments
- Consider using containerization (Docker) for consistent build environments

---

## EU CRA Compliance

This dependencies documentation supports EU Cyber Resilience Act compliance by:

1. **Complete Inventory**: All dependencies are documented
2. **Licensing Information**: All licenses are identified
3. **Security Monitoring**: Vulnerability tracking resources provided
4. **Update Procedures**: Clear update and patching guidance
5. **SBOM Integration**: Mirrors information in [SBOM.json](SBOM.json)

For more information on Puma's EU CRA compliance, see:
- [SECURITY.md](../SECURITY.md) - Security policy and vulnerability reporting
- [SBOM.json](SBOM.json) - Software Bill of Materials in SPDX format

---

## Contact

For questions about dependencies or security:
- Email: security@imagingtools.org
- GitHub Issues: https://github.com/ImagingTools/Puma/issues

---

## Updates to This Document

This document should be updated whenever:
- New dependencies are added
- Dependency versions change
- Security vulnerabilities are discovered
- License information changes

Last Updated: 2024-01-01
