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
	"0xacab.org/leap/vpnweb/pkg/metrics"
	"crypto/rand"
	"crypto/rsa"
	"crypto/tls"
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/pem"
	"io"
	"math/big"
	"time"
)

const keySize = 4096
const expiryDays = 120
const certPrefix = "BAZINGA"

type caInfo struct {
	cacrt, cakey string
}

func newCaInfo(cacrt string, cakey string) caInfo {
	return caInfo{cacrt, cakey}
}

// CertWriter main handler

func (ci *caInfo) CertWriter(out io.Writer) {
	catls, err := tls.LoadX509KeyPair(ci.cacrt, ci.cakey)

	if err != nil {
		panic(err)
	}
	ca, err := x509.ParseCertificate(catls.Certificate[0])
	if err != nil {
		panic(err)
	}
	serialNumberLimit := new(big.Int).Lsh(big.NewInt(1), 128)
	serialMin := new(big.Int).SetInt64(1000)
	serialNumber := new(big.Int).SetInt64(0)
	for serialNumber.Cmp(serialMin) < 0 {
		serialNumber, err = rand.Int(rand.Reader, serialNumberLimit)
	}

	subjectKeyID := make([]byte, 20)
	rand.Read(subjectKeyID)

	// Prepare certificate
	cert := &x509.Certificate{
		SerialNumber: serialNumber,

		Subject: pkix.Name{
			CommonName: certPrefix,
		},
		NotBefore: time.Now().AddDate(0, 0, -7),
		NotAfter:  time.Now().AddDate(0, 0, expiryDays),

		SubjectKeyId: subjectKeyID,

		ExtKeyUsage: []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth},
		KeyUsage:    x509.KeyUsageDigitalSignature,
	}
	priv, _ := rsa.GenerateKey(rand.Reader, keySize)
	pub := &priv.PublicKey

	// Sign the certificate
	certB, err := x509.CreateCertificate(rand.Reader, cert, ca, pub, catls.PrivateKey)

	// Write the private Key
	pem.Encode(out, &pem.Block{Type: "RSA PRIVATE KEY", Bytes: x509.MarshalPKCS1PrivateKey(priv)})

	// Write the public key
	pem.Encode(out, &pem.Block{Type: "CERTIFICATE", Bytes: certB})

	metrics.DownloadedCerts.Inc()
}
