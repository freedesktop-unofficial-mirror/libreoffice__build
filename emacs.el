(defvar libreoffice-path-regexp
  "libreoffice"
  "Regexp that matches paths into your LibreOffice source tree.")

(defun libreoffice-c-hook ()
  "Set C++ style and tab stops to match LO style."
  (when (string-match libreoffice-path-regexp buffer-file-name)
      (message "Using LibreOffice code settings")
      (setq tab-width 4)
      (setq tab-stop-list '(4 8 12 16 20 24 28 32 36 40 44 48 52 56 60 64 68 72
			    76 80 84 88 92 96 100 104 108 112 116 120))
      (c-set-style "stroustrup")
      (set-variable 'c-basic-offset 4)
      (set-variable 'indent-tabs-mode nil)
      (let ((env-file (libreoffice-set-env-file buffer-file-name)))
        (setq compile-command (if (null env-file)
                                  "dmake"
                                (concat ". "
                                        (file-relative-name env-file)
                                        "; dmake"))))))

(add-hook 'c-mode-common-hook 'libreoffice-c-hook)

(add-to-list 'auto-mode-alist (cons "\\.sdi\\'" 'c++-mode))
(add-to-list 'auto-mode-alist (cons "\\.hrc\\'" 'c++-mode))
(add-to-list 'auto-mode-alist (cons "\\.src\\'" 'c++-mode))

;; stolen from vc-hooks.el (vc-find-root)
(defun libreoffice-src-dir (file)
  "Find the root of an LibreOffice source directory.
Find the root of the LibreOffice source directory that
contains FILE, or nil if FILE is not in such a directory"
  (let ((dir nil))
    (while (not (or dir
                    (equal file (setq file (file-name-directory file)))
                    (null file)))
      (if (file-exists-p (expand-file-name "solenv" file))
          (setq dir file)
        (setq file (directory-file-name file))))
    dir))

(defun libreoffice-set-env-file (file)
  "Find LO *Env.Set.sh script.
Returns the name of the solar environment setter script for the
LibreOffice source tree to which FILE belongs. Unix-only."
  (let* ((root (libreoffice-src-dir file))
        (files (and root
                    (file-expand-wildcards
                     (expand-file-name "*Env.Set.sh" root)))))
    (if (null files) files (car files))))
