{% hint 'warning' %}
When an existing document is updated or replaced, ArangoDB will write a new
version of this document to the write-ahead logfile, regardless of the
storage engine. When the new version of the document has been written, the
old version(s) will still be present, at least on disk. The same is true when
an existing document (version) gets removed: for some time, the old version
of the document plus the removal operation will be on disk.

On disk it is therefore possible that many revisions of the same document
(as identified by the same `_key` value) exist at the same time.

However even if there might be multiple revisions of the same document (as
identified by the same `_key` value) somewhere on the disk, **they are not
accessible**. Once a document was updated or removed successfully, no query
or other data retrieval operation done by the users will be able to see it
any more. Furthermore, after some time, old revisions will be internally removed.
This is to avoid ever-growing disk usage.

Therefore, from a user perspective, there is just **one single document revision
present per different `_key` at every point in time**.
{% endhint %}