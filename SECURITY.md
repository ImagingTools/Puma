# Security Policy

## Supported Versions

The following versions of Puma are currently supported with security updates:

| Version | Supported          |
| ------- | ------------------ |
| Latest  | :white_check_mark: |
| < 1.0   | :x:                |

Please ensure you are using the latest version to receive security updates.

## Reporting a Vulnerability

We take the security of Puma seriously. If you discover a security vulnerability, please follow these guidelines:

### How to Report

**DO NOT** create a public GitHub issue for security vulnerabilities.

Instead, please report security vulnerabilities by emailing:

- **Email**: security@imagingtools.org (or contact repository maintainers directly)
- **Subject**: [SECURITY] Brief description of the vulnerability

### What to Include

Please include the following information in your report:

1. **Description**: A clear description of the vulnerability
2. **Impact**: The potential impact and severity of the vulnerability
3. **Steps to Reproduce**: Detailed steps to reproduce the vulnerability
4. **Affected Components**: Which components of Puma are affected (Server, Client, SDKs, etc.)
5. **Affected Versions**: Which versions are impacted
6. **Proposed Fix** (optional): If you have suggestions for fixing the vulnerability

### Response Timeline

- **Initial Response**: Within 48 hours of receiving the report
- **Status Update**: Weekly updates on the progress of addressing the vulnerability
- **Resolution**: Security patches will be released as soon as possible, typically within 30-90 days depending on severity and complexity

### Disclosure Policy

- We follow a coordinated disclosure policy
- We request that you do not publicly disclose the vulnerability until we have released a fix
- We will acknowledge your contribution in the security advisory (unless you prefer to remain anonymous)
- After the fix is released, we will publish a security advisory detailing the vulnerability and the fix

## Security Best Practices

When using Puma, we recommend following these security best practices:

### SSL/TLS Configuration

- Always use SSL/TLS for production deployments
- Use strong cipher suites and disable weak protocols (SSLv3, TLS 1.0, TLS 1.1)
- Keep OpenSSL libraries up to date
- Validate and use proper certificate management

### Authentication and Authorization

- Use strong passwords and enforce password policies
- Implement rate limiting to prevent brute force attacks
- Regularly audit user permissions and access logs
- Enable multi-factor authentication when available

### Database Security

- Use encrypted connections to databases when possible
- Follow the principle of least privilege for database access
- Regularly backup databases and test recovery procedures
- Keep SQLite and other database libraries up to date

### Dependency Management

- Regularly update all dependencies (Qt, OpenSSL, SQLite, ImtCore)
- Monitor security advisories for all dependencies
- Use dependency scanning tools to identify known vulnerabilities
- See [Docs/Dependencies.md](Docs/Dependencies.md) for a complete list of dependencies

### Server Deployment

- Run PumaServer with minimal required privileges
- Use firewalls to restrict network access
- Enable logging and monitoring for security events
- Keep the operating system and all system libraries patched

## Known Security Considerations

### Cryptographic Components

Puma relies on OpenSSL for cryptographic operations:
- Ensure OpenSSL is version 1.1.1 or later (3.x recommended)
- Verify that your OpenSSL build includes all necessary security patches
- Configure strong cipher suites in your server configuration

### WebSocket Security

Puma uses WebSockets for real-time communication:
- Always use WSS (WebSocket Secure) in production
- Implement proper authentication before establishing WebSocket connections
- Validate all WebSocket messages on the server side

### Database Security

SQLite is used for data persistence:
- Use appropriate file permissions to protect database files
- Consider encryption at rest for sensitive data
- Regularly backup database files securely

## Security Audits

We encourage security researchers and users to:
- Conduct security audits of the codebase
- Report any security concerns following our disclosure policy
- Contribute security improvements through pull requests

## Compliance

Puma is designed with security in mind and aims to comply with:
- EU Cyber Resilience Act (CRA)
- General Data Protection Regulation (GDPR) where applicable
- Industry best practices for authentication and authorization systems

For more information on our compliance efforts, see:
- [Software Bill of Materials (SBOM)](Docs/SBOM.json)
- [Dependencies Documentation](Docs/Dependencies.md)

## Updates and Notifications

To stay informed about security updates:
- Watch this repository for security advisories
- Check the [CHANGELOG](CHANGELOG.md) for security-related updates
- Subscribe to security notifications if available

## Contact

For security-related inquiries and reports:
- Email: security@imagingtools.org
- For non-security issues, please use the GitHub issue tracker

Thank you for helping keep Puma and its users secure!
