from typing import List, Set
from bs4 import BeautifulSoup
import requests

from Paths import ROOT_URL

def get_soup(url: str) -> BeautifulSoup:
    ''' Get BeautifulSoup for a url. '''
    response = requests.get(url)
    soup = BeautifulSoup(response.text, 'html.parser')
    return soup

def absolute_url(url: str) -> str:
    ''' Get the absolute url given any url. If relative, makes absolute with ROOT_URL. Otherwise,
        returns unmodified url. '''
    if url.startswith('/'):
        return ROOT_URL + url
    else:
        return url

def identify_states(start_url: str) -> Set[str]:
    ''' Identify states and returns a list of links to those states' pages. '''
    links = filter_links(get_links(get_soup(start_url)), ["state/"])
    return links

def identify_counties(start_url: str) -> Set[str]:
    ''' Identify counties and return a list of links to those counties' pages. '''
    links = filter_links(get_links(get_soup(start_url)), ["county/"])
    return links

def find_link_tree(start_url: str) -> Set[str]:
    ''' Get set of filtered links connected to the start url or its descendent pages. '''
    link_queue = [start_url]
    i: int = 0
    link: str = ""
    sublinks: Set[str] = set()
    while i < len(link_queue):
        print(f"index: {i} of {len(link_queue)}")
        link = link_queue[i]
        print("link: " + link)
        sublinks = filter_links(get_links(get_soup(link)))
        for sublink in sublinks:
            link_queue.append(sublink)
        i += 1
    return set(link_queue)

def get_links(soup: BeautifulSoup) -> List[str]:
    ''' Get all links (<a href=""> tags) on the page. '''
    links: List[str] = [absolute_url(link.get('href')) for link in soup.find_all('a') if link.get('href')] # type: ignore
    return links

_matches = ["state/"]
def filter_links(links: List[str], matches: List[str] = _matches) -> Set[str]:
    ''' Filter links into set of links containing text in the matches list. '''
    filtered: List[str] = []
    for link in links:
        if link not in filtered:
            for match in matches:
                if match in link:
                    filtered.append(link)
                    break
    return set(filtered)