# Releasing

The archives and `SHA256SUMS.txt` both come out of one CI run. Nothing is built or
summed by hand, so a release is those artifacts plus a signature over the sums file.

1. Tag and push, and let the build finish green.

2. Download the run's artifacts:

   ```bash
   gh run download <run-id> -D dist
   ```

3. Sign the sums file. This happens here, not in CI. A signing key kept in CI secrets
   could be used by anyone who compromised the repository, which would make the
   signature worth nothing.

   ```bash
   cd dist
   gpg --armor --detach-sign SHA256SUMS.txt
   ```

4. Publish the archives, `SHA256SUMS.txt` and `SHA256SUMS.txt.asc` together:

   ```bash
   gh release create <tag> --draft xmrig-* SHA256SUMS.txt SHA256SUMS.txt.asc
   ```

5. Check the draft: every archive present, the sums file listing all of them, and the
   signature verifying against the published sums file.

   ```bash
   gpg --verify SHA256SUMS.txt.asc SHA256SUMS.txt
   sha256sum -c SHA256SUMS.txt
   ```

6. Publish the draft.

Signing key: `5C2C FA03 0397 FCD7 63F1  A97B F878 8EFB 40E7 50E5`

If signing prompts fail with an ioctl error, the shell has no tty for the passphrase
prompt. `export GPG_TTY=$(tty)` fixes it.
