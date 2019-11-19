package leetcode

// 一个 AC 自动机的 Golang 语言范本
// KMP 的 DFA 是立刻能知道当前状态输入字符后下一个状态是什么
// 不像 AC 自动机还要一层层找

// AC 自动机 fail 代表有前缀的依次看 [p1 p2 p3 ... pj-1]+pj [p2 p3 ... pj-1]+pj [p3 ... pj-1]+pj
// 等到自动机的什么位置和没有位置的直接到 root
// search 要 fail 向上找跟找 fail 类似

// 一定要从自动机角度理解 当发现不匹配的时候，就把 [p1 p2 p3 ... pj-1]+pj 输入自动机，还不匹配，依次迭代
// 把 [p2 p3 ... pj-1]+pj 输入自动机 把[p3 ... pj-1]+pj 输入自动机

import (
	"log"
)

type Node struct {
	C        byte
	Parent   *Node
	NextList []*Node
	NextMap  [0x100]*Node
	Fail     *Node
	Off      int // off from root, start from 0
	Ended    bool
}

type Trie struct {
	Root *Node
}

func NewTrie() *Trie {
	return &Trie{
		Root: &Node{
			NextMap:  [256]*Node{},
			NextList: make([]*Node, 0),
		},
	}
}

func (t *Trie) insert(words []byte) {
	p := t.Root
	for off, c := range words {
		i := int(c)
		if p.NextMap[i] == nil {
			q := &Node{
				C:        c,
				Parent:   p,
				NextMap:  [256]*Node{},
				NextList: make([]*Node, 0),
				Off:      off,
			}
			p.NextMap[i] = q
			p.NextList = append(p.NextList, q)
		}
		p = p.NextMap[i]
	}
	p.Ended = true
}

func (t *Trie) build() {
	root := t.Root
	queue := make([]*Node, 0)
	for _, p := range root.NextList {
		p.Fail = root
		queue = append(queue, p)
	}
	for len(queue) > 0 {
		parent := queue[0]
		queue = queue[1:]
		for _, p := range parent.NextList {
			fail := parent.Fail
			idx := int(p.C)
			for fail != nil {
				if fail.NextMap[idx] != nil {
					p.Fail = fail.NextMap[idx]
					break
				}
				fail = fail.Fail
			}
			if fail == nil {
				p.Fail = t.Root
			}
			queue = append(queue, p)
		}
	}
}

// callback for tell matched [first, last)
// callback return true for not continue match
//    return false for continue match
func (t *Trie) search(target []byte, callback func(first int, last int) bool) {
	p := t.Root
	for idx, c := range target {
		for p.NextMap[c] == nil && p != t.Root {
			p = p.Fail
		}
		if p.NextMap[c] == nil {
			continue
		}
		p = p.NextMap[c]
		if p.Ended {
			if callback(idx-p.Off, idx+1) {
				break
			}
		}
	}
}

func ahoCorasick(s string, wordDict []string) bool {
	t := NewTrie()
	for _, str := range wordDict {
		t.insert([]byte(str))
	}
	t.build()
	sBytes := []byte(s)
	matched := false
	t.search(sBytes, func(first int, last int) bool {
		log.Printf("matched at [%v, %v) %v", first, last, string(sBytes[first:last]))
		matched = true
		return false
	})
	return matched
}

func t1() {
	ret := ahoCorasick("leetleetcode", []string{
		"leet",
		"code",
	})
	_ = ret
}

func t2() {
	ret := ahoCorasick("abcsfaaasdf", []string{
		"abc",
		"aaa",
		"bab",
	})
	_ = ret
}

func main() {
	t2()
}
