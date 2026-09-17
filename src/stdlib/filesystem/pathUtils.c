
#include "./pathUtils.h"

#include "stdlib/strings/StringBuffer.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// MARK: dirname
char *Path__dirname(const char *inPath)
{
  if (inPath == NULL)
  {
    return NULL;
  }

  const unsigned int len = strlen(inPath);
  if (len == 0)
  {
    return strndup(".", 1);
  }

  unsigned int cut = len;
  int trailing = 1;
  for (unsigned int ii = len - 1; ii >= 1; --ii)
  {
    if (inPath[ii] == '/')
    {
      if (!trailing)
      {
        cut = ii;
        break;
      }
    }
    else
    {
      trailing = 0;
    }
  }

  const int has_root = (inPath[0] == '/');

  if (cut == len)
  {
    if (has_root)
    {
      return strndup("/", 1);
    }
    return strndup(".", 1);
  }

  if (has_root && cut == 1)
  {
    return strndup("//", 2);
  }

  return strndup(inPath, cut);
}

// MARK: basename
char *Path__basename(const char *inPath)
{
  if (inPath == NULL)
  {
    return NULL;
  }

  unsigned int len = strlen(inPath);

  // Skip trailing "/"
  unsigned int end = len;
  while (end > 0 && inPath[end - 1] == '/')
  {
    --end;
  }

  // Only "/" or "//" -> empty basename
  if (end == 0)
  {
    return strndup("", 0);
  }

  // backwards scan for a "/"
  unsigned int start = end;
  while (start > 0 && inPath[start - 1] != '/')
  {
    --start;
  }

  return strndup(inPath + start, end - start);
}

// MARK; extname
char *Path__extname(const char *inPath)
{
  if (inPath == NULL)
  {
    return NULL;
  }

  const unsigned int len = strlen(inPath);

  // Skip the trailing run of "/" separators: no last component.
  unsigned int end = len;
  while (end > 0 && inPath[end - 1] == '/')
  {
    --end;
  }
  if (end == 0)
  {
    return strndup("", 0);
  }

  /* Locate the first character of the last component. */
  unsigned int start = end;
  while (start > 0 && inPath[start - 1] != '/')
  {
    --start;
  }

  /* Track the last "." in the component (start if none). */
  unsigned int dot = start;
  for (unsigned int ii = start; ii < end; ++ii)
  {
    if (inPath[ii] == '.')
    {
      dot = ii;
    }
  }

  /* No dot, a dot only at the component's first character (".env",
   * "."), or the component is exactly "..": no extension. */
  if (
      dot == start ||
      (dot == start + 1 && inPath[start] == '.' && end == start + 2))
  {
    return strndup("", 0);
  }

  return strndup(inPath + dot, end - dot);
}

// MARK: _normalize_string
static int _normalize_string(
    StringBuffer *inRes,
    const char *inPath,
    const int allowAboveRoot)
{
  const unsigned int len = strlen(inPath);
  unsigned int lastSegmentLength = 0;
  long lastSlash = -1;
  int dots = 0;
  char code = '\0';

  for (unsigned int ii = 0; ii <= len; ++ii)
  {
    if (ii < len)
    {
      code = inPath[ii];
    }
    else if (code == '/')
    {
      break;
    }
    else
    {
      code = '/';
    }

    if (code == '/')
    {
      if (lastSlash == (long)ii - 1 || dots == 1)
      {
        // empty or "." segment -> no-op
      }
      else if (dots == 2)
      {
        int pop_segment =
            (inRes->len < 2 || lastSegmentLength != 2 ||
             inRes->data[inRes->len - 1] != '.' ||
             inRes->data[inRes->len - 2] != '.');

        if (pop_segment)
        {
          if (inRes->len > 2)
          {
            /* Index of the '/' that begins the last segment. */
            long lsi = (long)inRes->len - (long)lastSegmentLength - 1;
            long last_idx;
            if (lsi == -1)
            {
              inRes->len = 0;
              lastSegmentLength = 0;
            }
            else
            {
              inRes->len = (unsigned int)lsi;
              last_idx = -1;
              for (long k = (long)inRes->len - 1; k >= 0; --k)
              {
                if (inRes->data[k] == '/')
                {
                  last_idx = k;
                  break;
                }
              }
              lastSegmentLength =
                  (unsigned int)((long)inRes->len - 1 - last_idx);
            }
            lastSlash = (long)ii;
            dots = 0;
            continue;
          }
          else if (inRes->len != 0)
          {
            inRes->len = 0;
            lastSegmentLength = 0;
            lastSlash = (long)ii;
            dots = 0;
            continue;
          }
        }
        if (allowAboveRoot)
        {
          if (inRes->len > 0)
          {
            if (StringBuffer__appendChar(inRes, '/') != 0)
            {
              return -1;
            }
            if (StringBuffer__appendData(inRes, 2, "..") != 0)
            {
              return -1;
            }
          }
          else
          {
            if (StringBuffer__appendData(inRes, 2, "..") != 0)
            {
              return -1;
            }
          }
          lastSegmentLength = 2;
        }
      }
      else
      {
        if (inRes->len > 0)
        {
          if (StringBuffer__appendChar(inRes, '/') != 0)
          {
            return -1;
          }
        }
        if (StringBuffer__appendData(inRes, ii - (unsigned int)(lastSlash + 1), inPath + (unsigned int)(lastSlash + 1)) != 0)
        {
          return -1;
        }
        lastSegmentLength = ii - (unsigned int)(lastSlash + 1);
      }
      lastSlash = (long)ii;
      dots = 0;
    }
    else if (code == '.' && dots != -1)
    {
      ++dots;
    }
    else
    {
      dots = -1;
    }
  }
  return 0;
}

// MARK: normalize
char *Path__normalize(const char *joined)
{
  unsigned int len = strlen(joined);
  if (len == 0)
  {
    return strndup(".", 1);
  }

  const int is_absolute = (joined[0] == '/');
  const int trailing_separator = (joined[len - 1] == '/');

  StringBuffer *res = StringBuffer__create();
  if (!res)
  {
    return NULL;
  }

  char *out = NULL;
  if (_normalize_string(res, joined, !is_absolute) != 0)
  {
    goto any_result_cleanup;
  }

  if (res->len == 0)
  {
    if (is_absolute)
    {
      out = strndup("/", 1);
    }
    else if (trailing_separator)
    {
      out = strndup("./", 2);
    }
    else
    {
      out = strndup(".", 1);
    }
    goto any_result_cleanup;
  }

  {
    unsigned int outlen = res->len;
    unsigned int pos = 0;
    if (trailing_separator)
    {
      ++outlen;
    }
    if (is_absolute)
    {
      ++outlen;
    }
    out = calloc(outlen + 1, sizeof(char));
    if (out == NULL)
    {
      goto any_result_cleanup;
    }
    if (is_absolute)
    {
      out[pos++] = '/';
    }
    memcpy(out + pos, res->data, res->len);
    pos += res->len;
    if (trailing_separator)
    {
      out[pos++] = '/';
    }
    out[pos] = '\0';
  }

any_result_cleanup:
  StringBuffer__free(&res);
  return out;
}

// MARK: join
char *Path__join(unsigned int n, ...)
{

  if (n == 0)
  {
    return strndup(".", 1);
  }

  StringBuffer *joined = StringBuffer__create();
  if (!joined)
  {
    return NULL;
  }

  va_list ap;
  va_start(ap, n);
  for (unsigned int i = 0; i < n; ++i)
  {
    const char *arg = va_arg(ap, const char *);
    if (arg == NULL)
    {
      goto va_failure_cleanup;
    }
    if (arg[0] == '\0')
    {
      continue; // ignored empty segments
    }
    if (joined->len > 0 && StringBuffer__appendChar(joined, '/') != 0)
    {
      goto va_failure_cleanup;
    }
    if (StringBuffer__appendCString(joined, arg) != 0)
    {
      goto va_failure_cleanup;
    }
  }
  va_end(ap);

  if (joined->len == 0)
  {
    StringBuffer__free(&joined);
    return strndup(".", 1);
  }

  char *result = Path__normalize(joined->data);
  StringBuffer__free(&joined);
  return result;

va_failure_cleanup:
  StringBuffer__free(&joined);
  va_end(ap);
  return NULL;
}

// MARK: _path_resolve_abs
static char *_path_resolve_abs(const char *path)
{
  StringBuffer *joined = StringBuffer__create();
  StringBuffer *res = NULL;

  if (joined == NULL)
  {
    return NULL;
  }

  if (path[0] == '\0')
  {
    path = ".";
  }
  if (path[0] != '/')
  {
    if (StringBuffer__appendChar(joined, '/') != 0)
    {
      goto failure_cleanup;
    }
  }
  if (StringBuffer__appendCString(joined, path) != 0)
  {
    goto failure_cleanup;
  }

  res = StringBuffer__create();
  if (res == NULL)
  {
    goto failure_cleanup;
  }
  if (_normalize_string(res, joined->data, 0) != 0)
  {
    goto failure_cleanup;
  }
  StringBuffer__free(&joined);

  if (res->len == 0)
  {
    StringBuffer__free(&res);
    return strndup("/", 1);
  }

  char *out = calloc(res->len + 2, sizeof(char));
  if (out == NULL)
  {
    goto failure_cleanup;
  }
  out[0] = '/';
  memcpy(out + 1, res->data, res->len);
  out[res->len + 1] = '\0';
  StringBuffer__free(&res);
  return out;

failure_cleanup:
  StringBuffer__free(&joined);
  StringBuffer__free(&res);
  return NULL;
}

// MARK: Path__relative
char *Path__relative(const char *from, const char *to)
{
  if (from == NULL || to == NULL)
  {
    return NULL;
  }
  if (strcmp(from, to) == 0)
  {
    return strndup("", 0);
  }

  char *from_abs = _path_resolve_abs(from);
  char *to_abs = _path_resolve_abs(to);
  if (from_abs == NULL || to_abs == NULL)
  {
    free(from_abs);
    free(to_abs);
    return NULL;
  }
  if (strcmp(from_abs, to_abs) == 0)
  {
    free(from_abs);
    free(to_abs);
    return strndup("", 0);
  }

  unsigned int from_end = strlen(from_abs);
  unsigned int to_end = strlen(to_abs);

  /* Walk both resolved paths; `lastSlash` tracks the last '/' shared by
   * them. If one path ends exactly at a '/' in the other, that position
   * is the boundary (an exact base directory); otherwise the boundary is
   * the last shared '/'. */
  unsigned int boundary = 0;
  unsigned int lastSlash = 0;
  unsigned int ii = 0;
  while (ii < from_end && ii < to_end && from_abs[ii] == to_abs[ii])
  {
    if (from_abs[ii] == '/')
    {
      lastSlash = ii;
    }
    ++ii;
  }
  if (from_abs[ii] == '\0' && ii < to_end && to_abs[ii] == '/')
  {
    // "from" = "to" base directory
    boundary = ii;
  }
  else if (to_abs[ii] == '\0' && ii < from_end && from_abs[ii] == '/')
  {
    // "to" = "from" base directory
    boundary = ii;
  }
  else
  {
    boundary = lastSlash;
  }

  /* Count the `from` segments that remain after the boundary: each
   * separator completes one, and a non-empty tail completes the last. */
  unsigned int ups = 0;
  for (ii = boundary + 1; ii < from_end; ++ii)
  {
    if (from_abs[ii] == '/')
    {
      ++ups;
    }
  }
  if (from_end > boundary + 1)
  {
    ++ups;
  }

  /* The `to` tail after the boundary, without its leading '/'. */
  unsigned int tail_len;
  unsigned int tail_start = boundary + 1;
  if (tail_start < to_end && to_abs[tail_start] == '/')
  {
    ++tail_start;
  }
  if (to_end > tail_start)
  {
    tail_len = to_end - tail_start;
  }
  else
  {
    tail_len = 0;
  }

  StringBuffer *out = StringBuffer__create();
  if (out == NULL)
  {
    free(from_abs);
    free(to_abs);
    return NULL;
  }

  char *result = NULL;

  for (ii = 0; ii < ups; ++ii)
  {
    if (ii > 0 && StringBuffer__appendChar(out, '/') != 0)
    {
      goto failure_cleanup;
    }
    if (StringBuffer__appendData(out, 2, "..") != 0)
    {
      goto failure_cleanup;
    }
  }
  if (ups > 0 && tail_len > 0 && StringBuffer__appendChar(out, '/') != 0)
  {
    goto failure_cleanup;
  }
  if (tail_len > 0 &&
      StringBuffer__appendData(out, tail_len, to_abs + tail_start) != 0)
  {
    goto failure_cleanup;
  }

  result = strndup(out->data, out->len);

failure_cleanup:
  StringBuffer__free(&out);
  free(from_abs);
  free(to_abs);
  return result;
}
