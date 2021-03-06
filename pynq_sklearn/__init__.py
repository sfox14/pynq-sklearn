
from .base import PynqMixin
from .register import HybridLibrary, Registry
from . import linear_model, random_projection

__author__ = "Sean Fox"
__version__ = 0.1

__all__ = ['linear_model',
          'random_projection']