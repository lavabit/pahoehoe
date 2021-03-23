// Package main Example uses net.Http over a TURN connection. Expects server to
// support RFC 6062 (TURN extensions for TCP).
package main

import (
	"encoding/json"
	"flag"
	"github.com/willscott/goturn/client"
	"github.com/willscott/goturn/common"
	"io/ioutil"
	"log"
	"net"
	"net/http"
	"net/url"
)

var webpage = flag.String("url", "https://wtfismyip.com/text", "URL to fetch")
var credentialURL = flag.String("credentials", "https://computeengineondemand.appspot.com/turn?username=prober&key=4080218913", "credential URL")

type Credentials struct {
	Username string   `json:"username"`
	Password string   `json:"password"`
	Uris     []string `json:"uris"`
}

func main() {
	flag.Parse()

	// get & parse credentials
	resp, err := http.Get(*credentialURL)
	if err != nil {
		log.Fatal("Could not fetch URL:", err)
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Fatal("Could not read response:", err)
	}

	var creds Credentials
	if err := json.Unmarshal(body, &creds); err != nil {
		log.Fatal("Could not parse response:", err)
	}

	// Use the first one.
	server, err := url.Parse(creds.Uris[0])
	if err != nil {
		log.Fatal("Invalid server URI:", err)
	}

	webpageUrl, err := url.Parse(*webpage)
	if err != nil {
		log.Fatal("Invalid request URI:", err)
	}
	webpageHost, err := net.LookupIP(webpageUrl.Host)
	if err != nil {
		log.Fatal("Could not determine host IP of "+webpageUrl.Host, err)
	}
	webpagePort := uint16(80)
	if webpageUrl.Scheme == "https" {
		webpagePort = uint16(443)
	}

	turnaddr, err := net.ResolveTCPAddr("tcp", server.Opaque)
	if err != nil {
		log.Fatal("Could resolve remote address:", err)
	}
	log.Printf("Negotiating with %s", server.Opaque)

	// dial
	c, err := net.Dial("tcp", turnaddr.String())
	if err != nil {
		log.Fatal("Could open TCP Connection:", err)
	}
	defer c.Close()

	client := client.StunClient{Conn: c}
	credentials := stun.Credentials{Username: creds.Username, Password: creds.Password}
	if _, err = client.Allocate(&credentials); err != nil {
		log.Fatal("Could not authenticate with server: ", err)
	}

	endpoint := stun.NewAddress("tcp", webpageHost[0], webpagePort)
	if err = client.RequestPermission(endpoint); err != nil {
		log.Fatal("Could not request permission:", err)
	}

	conn, err := client.Connect(endpoint)
	if err != nil {
		log.Fatal("Could not establish connection:", err)
	}

	log.Printf("Connection established.")

	dumbDialer := func(network, address string) (net.Conn, error) {
		return conn, nil
	}

	transport := &http.Transport{Dial: dumbDialer}
	httpClient := &http.Client{Transport: transport}
	httpResp, err := httpClient.Get(*webpage)
	if err != nil {
		log.Fatal("Failed to get webpage", err)
	}
	defer httpResp.Body.Close()
	httpBody, err := ioutil.ReadAll(httpResp.Body)
	if err != nil {
		log.Fatal("Failed to read response", err)
	}
	log.Printf("Received Webpage Body is: %s", string(httpBody))
}
