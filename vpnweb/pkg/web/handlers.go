// Copyright (C) 2019 LEAP
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

package web

import (
	"net/http"
)

type CertHandler struct {
	Cainfo caInfo
}

func NewCertHandler(caCrt, caKey string) CertHandler {
	ci := newCaInfo(caCrt, caKey)
	ch := CertHandler{ci}
	return ch
}

func (ch *CertHandler) CertResponder(w http.ResponseWriter, r *http.Request) {
	ch.Cainfo.CertWriter(w)
}

func HttpFileHandler(mux *http.ServeMux, route string, path string) {
	mux.HandleFunc(route, func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, path)
	})
}
